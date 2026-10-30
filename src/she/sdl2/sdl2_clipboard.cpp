// SHE library
// Aseprite  | Copyright (C) 2016 David Capello
// Besprited | Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "she/sdl2/sdl2_clipboard.h"

#include "gfx/size.h"
#include "she/display.h"
#include "she/system.h"

#include <cstring>

namespace she
{

namespace
{

void* nativeDisplayHandle()
{
  Display* display = instance()->defaultDisplay();
  return display ? display->nativeHandle() : nullptr;
}

// Extracts the 8-bit channel selected by mask/shift.
inline uint8_t channel(uint64_t c, unsigned long mask, unsigned long shift, int extra = 0)
{
  return uint8_t((c & mask) >> shift >> extra);
}

inline uint8_t scaleBits(unsigned v, int bits)
{
  return uint8_t((v << (8 - bits)) | (v >> (2 * bits - 8)));
}

} // namespace

SDL2Clipboard::SDL2Clipboard()
  : m_customFormat(clip::register_format("application/x-besprited-image"))
{
}

bool SDL2Clipboard::getText(std::string& text)
{
  return clip::get_text(text);
}

bool SDL2Clipboard::setText(const std::string& text)
{
  return clip::set_text(text);
}

bool SDL2Clipboard::hasImage()
{
  return clip::has(clip::image_format());
}

bool SDL2Clipboard::getImageSize(gfx::Size& size)
{
  clip::image_spec spec;
  if (!clip::get_image_spec(spec))
    return false;
  size.w = spec.width;
  size.h = spec.height;
  return true;
}

bool SDL2Clipboard::hasCustomData()
{
  return clip::has(m_customFormat);
}

bool SDL2Clipboard::getCustomData(std::vector<uint8_t>& data)
{
  clip::lock l(nativeDisplayHandle());
  if (!l.locked() || !l.is_convertible(m_customFormat))
    return false;
  const size_t size = l.get_data_length(m_customFormat);
  if (size == 0)
    return false;
  data.resize(size);
  return l.get_data(m_customFormat, reinterpret_cast<char*>(data.data()), size);
}

bool SDL2Clipboard::getImage(ClipboardImage& image)
{
  clip::lock l(nativeDisplayHandle());
  if (!l.locked() || !l.is_convertible(clip::image_format()))
    return false;

  clip::image img;
  if (!l.get_image(img))
    return false;

  const clip::image_spec& spec = img.spec();
  image.width = spec.width;
  image.height = spec.height;
  image.rgba.assign(static_cast<size_t>(spec.width) * spec.height * 4, 255);
  uint8_t* dst = image.rgba.data();

  for (unsigned long y = 0; y < spec.height; ++y)
  {
    const char* row = img.data() + spec.bytes_per_row * y;
    for (unsigned long x = 0; x < spec.width; ++x, dst += 4)
    {
      switch (spec.bits_per_pixel)
      {
      case 64:
      {
        uint64_t c;
        std::memcpy(&c, row + x * 8, 8);
        dst[0] = channel(c, spec.red_mask, spec.red_shift, 8);
        dst[1] = channel(c, spec.green_mask, spec.green_shift, 8);
        dst[2] = channel(c, spec.blue_mask, spec.blue_shift, 8);
        dst[3] = channel(c, spec.alpha_mask, spec.alpha_shift, 8);
        break;
      }
      case 32:
      {
        uint32_t c;
        std::memcpy(&c, row + x * 4, 4);
        dst[0] = channel(c, spec.red_mask, spec.red_shift);
        dst[1] = channel(c, spec.green_mask, spec.green_shift);
        dst[2] = channel(c, spec.blue_mask, spec.blue_shift);
        // On Windows, 32bpp images have no alpha information.
        dst[3] = spec.alpha_mask ? channel(c, spec.alpha_mask, spec.alpha_shift) : 255;
        break;
      }
      case 24:
      {
        uint32_t c = 0;
        std::memcpy(&c, row + x * 3, 3);
        dst[0] = channel(c, spec.red_mask, spec.red_shift);
        dst[1] = channel(c, spec.green_mask, spec.green_shift);
        dst[2] = channel(c, spec.blue_mask, spec.blue_shift);
        break;
      }
      case 16:
      {
        uint16_t c;
        std::memcpy(&c, row + x * 2, 2);
        dst[0] = scaleBits((c & spec.red_mask) >> spec.red_shift, 5);
        dst[1] = scaleBits((c & spec.green_mask) >> spec.green_shift, 6);
        dst[2] = scaleBits((c & spec.blue_mask) >> spec.blue_shift, 5);
        break;
      }
      default:
        return false;
      }
    }
  }
  return true;
}

bool SDL2Clipboard::setImage(const ClipboardImage& image, const std::vector<uint8_t>* customData)
{
  if (image.width <= 0 || image.height <= 0 || image.rgba.size() < static_cast<size_t>(image.width) * image.height * 4)
    return false;

  clip::lock l(nativeDisplayHandle());
  if (!l.locked())
    return false;
  l.clear();

  if (m_customFormat && customData && !customData->empty())
    l.set_data(m_customFormat, reinterpret_cast<const char*>(customData->data()), customData->size());

  clip::image_spec spec;
  spec.width = image.width;
  spec.height = image.height;
  spec.bits_per_pixel = 32;
  spec.bytes_per_row = 4 * spec.width;
  spec.red_mask = 0xff;
  spec.green_mask = 0xff00;
  spec.blue_mask = 0xff0000;
  spec.alpha_mask = 0xff000000;
  spec.red_shift = 0;
  spec.green_shift = 8;
  spec.blue_shift = 16;
  spec.alpha_shift = 24;

  clip::image img(image.rgba.data(), spec);
  return l.set_image(img);
}

} // namespace she
