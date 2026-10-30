// SHE library
// Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "she/sdl3/sdl3_clipboard.h"

#include "gfx/size.h"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <cstring>
#include <memory>

namespace she
{

namespace
{

constexpr const char* kPngMime = "image/png";
// Other formats are accepted on paste only (IMG_Load_IO detects the type).
constexpr const char* kPasteMimes[] = {kPngMime, "image/bmp", "image/jpeg", "image/webp", "image/tiff"};
constexpr const char* kCustomMime = "application/x-besprited-image";

struct SdlFree
{
  void operator()(void* p) const { SDL_free(p); }
};

struct SurfaceDeleter
{
  void operator()(SDL_Surface* s) const { SDL_DestroySurface(s); }
};

using SdlBuffer = std::unique_ptr<void, SdlFree>;
using SurfacePtr = std::unique_ptr<SDL_Surface, SurfaceDeleter>;

// Held by SDL until another clipboard owner replaces ours. The PNG is
// encoded lazily, only when some application actually asks for it.
struct Offer
{
  ClipboardImage image;
  std::vector<uint8_t> custom;
  bool hasCustom = false;
  std::vector<uint8_t> png;
  bool pngEncoded = false;

  void encodePng()
  {
    if (pngEncoded)
      return;
    pngEncoded = true;

    SurfacePtr surface{
        SDL_CreateSurfaceFrom(image.width, image.height, SDL_PIXELFORMAT_RGBA32, image.rgba.data(), image.width * 4)};
    if (!surface)
      return;

    SDL_IOStream* io = SDL_IOFromDynamicMem();
    if (!io)
      return;
    if (IMG_SavePNG_IO(surface.get(), io, false))
    {
      const auto size = static_cast<size_t>(SDL_TellIO(io));
      const auto* mem = static_cast<const uint8_t*>(
          SDL_GetPointerProperty(SDL_GetIOProperties(io), SDL_PROP_IOSTREAM_DYNAMIC_MEMORY_POINTER, nullptr));
      if (mem)
        png.assign(mem, mem + size);
    }
    SDL_CloseIO(io);
  }
};

const void* offerCallback(void* userdata, const char* mimeType, size_t* size)
{
  auto* offer = static_cast<Offer*>(userdata);
  *size = 0;
  if (!mimeType)
    return nullptr;

  const std::vector<uint8_t>* data = nullptr;
  if (std::strcmp(mimeType, kCustomMime) == 0 && offer->hasCustom)
    data = &offer->custom;
  else if (std::strcmp(mimeType, kPngMime) == 0)
  {
    offer->encodePng();
    data = &offer->png;
  }

  if (!data || data->empty())
    return nullptr;
  *size = data->size();
  return data->data();
}

void offerCleanup(void* userdata)
{
  delete static_cast<Offer*>(userdata);
}

bool getBlob(const char* mime, std::vector<uint8_t>& out)
{
  size_t size = 0;
  SdlBuffer buf{SDL_GetClipboardData(mime, &size)};
  if (!buf || size == 0)
    return false;
  const auto* p = static_cast<const uint8_t*>(buf.get());
  out.assign(p, p + size);
  return true;
}

SurfacePtr decodePng()
{
  size_t size = 0;
  SdlBuffer buf;
  for (const char* mime : kPasteMimes)
  {
    if (!SDL_HasClipboardData(mime))
      continue;
    buf.reset(SDL_GetClipboardData(mime, &size));
    if (buf && size > 0)
      break;
  }
  if (!buf || size == 0)
    return nullptr;

  SDL_IOStream* io = SDL_IOFromConstMem(buf.get(), size);
  if (!io)
    return nullptr;
  return SurfacePtr{IMG_Load_IO(io, true)};
}

} // namespace

bool SDL3Clipboard::getText(std::string& text)
{
  if (!SDL_HasClipboardText())
    return false;
  SdlBuffer buf{SDL_GetClipboardText()};
  if (!buf)
    return false;
  text = static_cast<const char*>(buf.get());
  return true;
}

bool SDL3Clipboard::setText(const std::string& text)
{
  return SDL_SetClipboardText(text.c_str());
}

bool SDL3Clipboard::hasImage()
{
  for (const char* mime : kPasteMimes)
    if (SDL_HasClipboardData(mime))
      return true;
  return false;
}

bool SDL3Clipboard::getImageSize(gfx::Size& size)
{
  if (!hasImage())
    return false;
  SurfacePtr surface = decodePng();
  if (!surface)
    return false;
  size.w = surface->w;
  size.h = surface->h;
  return true;
}

bool SDL3Clipboard::getImage(ClipboardImage& image)
{
  if (!hasImage())
    return false;
  SurfacePtr surface = decodePng();
  if (!surface)
    return false;

  SurfacePtr rgba{SDL_ConvertSurface(surface.get(), SDL_PIXELFORMAT_RGBA32)};
  if (!rgba)
    return false;

  image.width = rgba->w;
  image.height = rgba->h;
  image.rgba.resize(static_cast<size_t>(rgba->w) * rgba->h * 4);
  const auto* src = static_cast<const uint8_t*>(rgba->pixels);
  for (int y = 0; y < rgba->h; ++y)
    std::memcpy(&image.rgba[static_cast<size_t>(y) * rgba->w * 4], src + static_cast<size_t>(y) * rgba->pitch,
                static_cast<size_t>(rgba->w) * 4);
  return true;
}

bool SDL3Clipboard::hasCustomData()
{
  return SDL_HasClipboardData(kCustomMime);
}

bool SDL3Clipboard::getCustomData(std::vector<uint8_t>& data)
{
  return hasCustomData() && getBlob(kCustomMime, data);
}

bool SDL3Clipboard::setImage(const ClipboardImage& image, const std::vector<uint8_t>* customData)
{
  if (image.width <= 0 || image.height <= 0 || image.rgba.size() < static_cast<size_t>(image.width) * image.height * 4)
    return false;

  auto offer = std::make_unique<Offer>();
  offer->image = image;
  if (customData && !customData->empty())
  {
    offer->custom = *customData;
    offer->hasCustom = true;
  }

  const char* mimes[2] = {kPngMime, kCustomMime};
  const size_t numMimes = offer->hasCustom ? 2 : 1;
  if (!SDL_SetClipboardData(offerCallback, offerCleanup, offer.get(), mimes, numMimes))
    return false;
  offer.release();
  return true;
}

} // namespace she
