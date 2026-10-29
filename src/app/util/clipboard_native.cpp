// Aseprite  | Copyright (C) 2016 David Capello
// Besprited | Copyright (C) 2026 Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/util/clipboard_native.h"

#include "base/serialization.h"
#include "doc/color_scales.h"
#include "doc/image.h"
#include "doc/image_impl.h"
#include "doc/image_io.h"
#include "doc/mask.h"
#include "doc/mask_io.h"
#include "doc/palette_io.h"
#include "gfx/size.h"
#include "she/clipboard.h"
#include "she/system.h"

#include <memory>
#include <sstream>
#include <vector>

namespace app
{
namespace clipboard
{

using namespace base::serialization;
using namespace base::serialization::little_endian;

namespace
{

she::Clipboard* native()
{
  return she::instance()->clipboard();
}

// Serializes image + mask + palette so a round trip through the clipboard
// doesn't lose them.
std::vector<uint8_t> serialize(const doc::Image* image, const doc::Mask* mask, const doc::Palette* palette)
{
  std::stringstream os;
  write32(os, 1 | (mask ? 2 : 0) | (palette ? 4 : 0));
  doc::write_image(os, image);
  if (mask)
    doc::write_mask(os, mask);
  if (palette)
    doc::write_palette(os, *palette);

  if (!os.good())
    return {};
  const std::string str = os.str();
  return {str.begin(), str.end()};
}

} // namespace

bool has_native_clipboard_bitmap()
{
  return native()->hasImage();
}

bool set_native_clipboard_bitmap(const doc::Image* image, const doc::Mask* mask, const doc::Palette* palette)
{
  if (!image)
    return false;

  she::ClipboardImage img;
  img.width = image->width();
  img.height = image->height();
  img.rgba.resize(static_cast<size_t>(img.width) * img.height * 4);
  uint8_t* dst = img.rgba.data();

  auto put = [&dst](doc::color_t c)
  {
    *dst++ = doc::rgba_getr(c);
    *dst++ = doc::rgba_getg(c);
    *dst++ = doc::rgba_getb(c);
    *dst++ = doc::rgba_geta(c);
  };

  switch (image->pixelFormat())
  {
  case doc::IMAGE_RGB:
  {
    const doc::LockImageBits<doc::RgbTraits> bits(image);
    for (auto it = bits.begin(), end = bits.end(); it != end; ++it)
      put(*it);
    break;
  }
  case doc::IMAGE_GRAYSCALE:
  {
    const doc::LockImageBits<doc::GrayscaleTraits> bits(image);
    for (auto it = bits.begin(), end = bits.end(); it != end; ++it)
    {
      const doc::color_t c = *it;
      put(doc::rgba(doc::graya_getv(c), doc::graya_getv(c), doc::graya_getv(c), doc::graya_geta(c)));
    }
    break;
  }
  case doc::IMAGE_INDEXED:
  {
    if (!palette)
      return false;
    const doc::LockImageBits<doc::IndexedTraits> bits(image);
    for (auto it = bits.begin(), end = bits.end(); it != end; ++it)
    {
      doc::color_t c = palette->getEntry(*it);

      // Use alpha=0 for mask color
      if (*it == image->maskColor())
        c &= doc::rgba_rgb_mask;

      put(c);
    }
    break;
  }
  default:
    return false;
  }

  const std::vector<uint8_t> custom = serialize(image, mask, palette);
  return native()->setImage(img, custom.empty() ? nullptr : &custom);
}

bool get_native_clipboard_bitmap(doc::Image** image, doc::Mask** mask, std::shared_ptr<doc::Palette>& palette)
{
  *image = nullptr;
  *mask = nullptr;
  palette.reset();

  she::Clipboard* cb = native();

  // Prefer the custom format (to avoid losing mask and palette)
  std::vector<uint8_t> custom;
  if (cb->getCustomData(custom))
  {
    std::stringstream is;
    is.write(reinterpret_cast<const char*>(custom.data()), custom.size());
    is.seekg(0);

    const int bits = read32(is);
    if (bits & 1)
      *image = doc::read_image(is, false);
    if (bits & 2)
      *mask = doc::read_mask(is);
    if (bits & 4)
      palette = doc::read_palette(is);
    if (*image)
      return true;

    // Unusable blob: discard the partial parse and fall back to the image
    delete *mask;
    *mask = nullptr;
    palette.reset();
  }

  she::ClipboardImage img;
  if (!cb->getImage(img))
    return false;

  std::unique_ptr<doc::Image> dst(doc::Image::create(doc::IMAGE_RGB, img.width, img.height));
  doc::LockImageBits<doc::RgbTraits> bits(dst.get(), doc::Image::WriteLock);
  const uint8_t* src = img.rgba.data();
  for (auto it = bits.begin(), end = bits.end(); it != end; ++it, src += 4)
    *it = doc::rgba(src[0], src[1], src[2], src[3]);

  *image = dst.release();
  return true;
}

bool get_native_clipboard_bitmap_size(gfx::Size* size)
{
  return native()->getImageSize(*size);
}

} // namespace clipboard
} // namespace app
