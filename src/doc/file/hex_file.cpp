// Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "base/fstream_path.h"
#include "base/trim_string.h"
#include "doc/color.h"
#include "doc/palette.h"

#include <cstdint>
#include <fstream>
#include <iomanip>
#include <memory>
#include <string>

namespace doc::file
{

namespace
{

int hex_digit(char c)
{
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  return -1;
}

bool parse_hex(const std::string& text, unsigned int& value)
{
  value = 0;
  for (char c : text)
  {
    const int digit = hex_digit(c);
    if (digit < 0)
      return false;
    value = (value << 4) | static_cast<unsigned int>(digit);
  }
  return true;
}

} // namespace

std::shared_ptr<Palette> load_hex_file(const char* filename)
{
  std::ifstream f(FSTREAM_PATH(filename), std::ios::binary);
  if (!f)
    return {};

  auto pal = Palette::create(0);
  std::string line;
  bool first = true;
  while (std::getline(f, line))
  {
    if (first && line.compare(0, 3, "\xEF\xBB\xBF") == 0)
      line.erase(0, 3);
    first = false;

    std::string text;
    base::trim_string(line, text);
    if (text.empty())
      continue;
    if (text.front() == '#')
      text.erase(0, 1);

    if (text.size() != 6 && text.size() != 8)
      return {};

    unsigned int rgbOrRgba;
    if (!parse_hex(text, rgbOrRgba))
      return {};
    const uint8_t r = static_cast<uint8_t>((rgbOrRgba >> (text.size() == 8 ? 24 : 16)) & 0xff);
    const uint8_t g = static_cast<uint8_t>((rgbOrRgba >> (text.size() == 8 ? 16 : 8)) & 0xff);
    const uint8_t b = static_cast<uint8_t>((rgbOrRgba >> (text.size() == 8 ? 8 : 0)) & 0xff);
    const uint8_t alpha = text.size() == 8 ? static_cast<uint8_t>(rgbOrRgba & 0xff) : 255;
    pal->addEntry(rgba(r, g, b, alpha));
  }

  if (f.bad() || pal->size() == 0)
    return {};
  return pal;
}

bool save_hex_file(const Palette& pal, const char* filename)
{
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
