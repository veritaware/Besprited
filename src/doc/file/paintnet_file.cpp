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

bool parse_hex_color(const std::string& text, uint32_t& value)
{
  if (text.size() != 8)
    return false;
  const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value, 16);
  return error == std::errc() && end == text.data() + text.size();
}

} // namespace

std::shared_ptr<Palette> load_paintnet_file(const char* filename)
{
  auto pal = Palette::create(0);
  const bool read = text_palette_file::read_lines(
      filename,
      [&](const std::string& text)
      {
        if (text.empty() || text.front() == ';')
          return true;

        uint32_t argb;
        if (!parse_hex_color(text, argb))
          return false;
        pal->addEntry(rgba((argb >> 16) & 0xff, (argb >> 8) & 0xff, argb & 0xff, (argb >> 24) & 0xff));
        return true;
      });

  if (!read || pal->size() == 0)
    return {};
  return pal;
}

bool save_paintnet_file(const Palette& pal, const char* filename)
{
  if (pal.size() == 0)
    return false;

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
