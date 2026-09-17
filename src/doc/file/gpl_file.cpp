// Document Library
// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "base/fstream_path.h"
#include "base/split_string.h"
#include "base/trim_string.h"
#include "doc/image.h"
#include "doc/palette.h"

#include <cctype>
#include <fstream>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>

namespace doc::file
{

std::shared_ptr<Palette> load_gpl_file(const char* filename)
{
  std::ifstream f(FSTREAM_PATH(filename));
  if (f.bad())
    return {};

  // Read first line, it must be "GIMP Palette"
  std::string line;
  if (!std::getline(f, line))
    return {};
  base::trim_string(line, line);
  if (line != "GIMP Palette")
    return {};

  auto pal = Palette::create(0);

  while (std::getline(f, line))
  {
    // Trim line.
    base::trim_string(line, line);

    // Remove comments
    if (line.empty() || line[0] == '#')
      continue;

    // Remove properties (TODO add these properties in the palette)
    if (!std::isdigit(line[0]))
      continue;

    int r, g, b;
    std::istringstream lineIn(line);
    // TODO add support to read the color name
    lineIn >> r >> g >> b;

    if (lineIn.fail())
      continue;

    pal->addEntry(rgba(r, g, b, 255));
  }

  return pal;
}

bool save_gpl_file(const Palette& pal, const char* filename)
{
  std::ofstream f(FSTREAM_PATH(filename));
  if (f.bad())
    return false;

  f << "GIMP Palette\n"
    << "#\n";

  for (int i = 0; i < pal.size(); ++i)
  {
    const uint32_t col = pal.getEntry(i);
    f << std::setfill(' ') << std::setw(3) << (static_cast<int>(rgba_getr(col)))
      << " " << std::setfill(' ') << std::setw(3)
      << (static_cast<int>(rgba_getg(col))) << " " << std::setfill(' ')
      << std::setw(3) << (static_cast<int>(rgba_getb(col))) << "\tUntitled\n";
  }

  return true;
}

} // namespace doc::file
