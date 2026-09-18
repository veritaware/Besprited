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

#include <memory>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>

namespace doc::file
{

std::shared_ptr<Palette> load_pal_file(const char* filename)
{
  std::ifstream f(FSTREAM_PATH(filename));
  if (f.bad())
    return nullptr;

  // Read first line, it must be "JASC-PAL"
  std::string line;
  if (!std::getline(f, line))
    return nullptr;
  base::trim_string(line, line);
  if (line != "JASC-PAL")
    return nullptr;

  // Second line is the version (0100)
  if (!std::getline(f, line))
    return nullptr;
  base::trim_string(line, line);
  if (line != "0100")
    return nullptr;

  // Ignore number of colors (we'll read line by line anyway)
  if (!std::getline(f, line))
    return nullptr;

  auto pal = Palette::create(0);

  while (std::getline(f, line))
  {
    // Trim line
    base::trim_string(line, line);

    // Remove comments
    if (line.empty())
      continue;

    int r, g, b;
    std::istringstream lineIn(line);
    lineIn >> r >> g >> b;
    pal->addEntry(rgba(r, g, b, 255));
  }

  return pal;
}

bool save_pal_file(const Palette& pal, const char* filename)
{
  std::ofstream f(FSTREAM_PATH(filename));
  if (f.bad())
    return false;

  f << "JASC-PAL\n"
    << "0100\n"
    << pal.size() << "\n";

  for (int i = 0; i < pal.size(); ++i)
  {
    const uint32_t col = pal.getEntry(i);
    f << (static_cast<int>(rgba_getr(col))) << " "
      << (static_cast<int>(rgba_getg(col))) << " "
      << (static_cast<int>(rgba_getb(col))) << "\n";
  }

  return true;
}

} // namespace doc::file
