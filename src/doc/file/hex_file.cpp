// Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "base/fstream_path.h"
#include "doc/color.h"
#include "doc/palette.h"
#include "doc/file/text_palette_file.h"

#include <charconv>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <memory>
#include <string>
#include <system_error>

namespace doc::file
{

namespace
{

bool parse_hex(const std::string& text, unsigned int& value)
{
  const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value, 16);
  return error == std::errc() && end == text.data() + text.size();
}

} // namespace

std::shared_ptr<Palette> load_hex_file(const char* filename)
{
  auto pal = Palette::create(0);
  const bool read = text_palette_file::read_lines(
      filename,
      [&](const std::string& line)
      {
        std::string text = line;
        if (text.empty())
          return true;
        if (text.front() == '#')
          text.erase(0, 1);

        if (text.size() != 6 && text.size() != 8)
          return false;

        unsigned int rgbOrRgba;
        if (!parse_hex(text, rgbOrRgba))
          return false;
        const uint8_t r = static_cast<uint8_t>((rgbOrRgba >> (text.size() == 8 ? 24 : 16)) & 0xff);
        const uint8_t g = static_cast<uint8_t>((rgbOrRgba >> (text.size() == 8 ? 16 : 8)) & 0xff);
        const uint8_t b = static_cast<uint8_t>((rgbOrRgba >> (text.size() == 8 ? 8 : 0)) & 0xff);
        const uint8_t alpha = text.size() == 8 ? static_cast<uint8_t>(rgbOrRgba & 0xff) : 255;
        pal->addEntry(rgba(r, g, b, alpha));
        return true;
      });

  if (!read || pal->size() == 0)
    return {};
  return pal;
}

bool save_hex_file(const Palette& pal, const char* filename)
{
  if (pal.size() == 0)
    return false;

  std::ofstream f(FSTREAM_PATH(filename), std::ios::binary);
  if (!f)
    return false;

  const bool withAlpha = pal.hasAlpha();
  for (int i = 0; i < pal.size(); ++i)
  {
    const color_t color = pal.entry(i);
    f << std::uppercase << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(rgba_getr(color))
      << std::setw(2) << static_cast<int>(rgba_getg(color)) << std::setw(2) << static_cast<int>(rgba_getb(color));
    if (withAlpha)
      f << std::setw(2) << static_cast<int>(rgba_geta(color));
    f << '\n';
  }
  return f.good();
}

} // namespace doc::file
