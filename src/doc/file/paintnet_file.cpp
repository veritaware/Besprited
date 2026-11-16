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

bool parse_hex_color(const std::string& text, uint32_t& value)
{
  if (text.size() != 8)
    return false;
  value = 0;
  for (char c : text)
  {
    const int digit = hex_digit(c);
    if (digit < 0)
      return false;
    value = (value << 4) | static_cast<uint32_t>(digit);
  }
  return true;
}

} // namespace

std::shared_ptr<Palette> load_paintnet_file(const char* filename)
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
    if (text.empty() || text.front() == ';')
      continue;

    uint32_t argb;
    if (!parse_hex_color(text, argb))
      return {};
    pal->addEntry(rgba((argb >> 16) & 0xff, (argb >> 8) & 0xff, argb & 0xff, (argb >> 24) & 0xff));
  }

  if (f.bad() || pal->size() == 0)
    return {};
  return pal;
}

bool save_paintnet_file(const Palette& pal, const char* filename)
{
  std::ofstream f(FSTREAM_PATH(filename), std::ios::binary);
  if (!f)
    return false;

  f << "; Paint.NET Palette File\n";
  for (int i = 0; i < pal.size(); ++i)
  {
    const color_t color = pal.entry(i);
    f << std::uppercase << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(rgba_geta(color))
      << std::setw(2) << static_cast<int>(rgba_getr(color)) << std::setw(2) << static_cast<int>(rgba_getg(color))
      << std::setw(2) << static_cast<int>(rgba_getb(color)) << '\n';
  }
  return f.good();
}

} // namespace doc::file
