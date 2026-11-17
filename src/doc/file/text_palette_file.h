// Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include "base/fstream_path.h"
#include "base/trim_string.h"

#include <fstream>
#include <string>

namespace doc::file
{
namespace text_palette_file
{

template <typename ReadLine> bool read_lines(const char* filename, ReadLine&& readLine)
{
  std::ifstream f(FSTREAM_PATH(filename), std::ios::binary);
  if (!f)
    return false;

  std::string line;
  bool first = true;
  while (std::getline(f, line))
  {
    if (first && line.compare(0, 3, "\xEF\xBB\xBF") == 0)
      line.erase(0, 3);
    first = false;

    std::string text;
    base::trim_string(line, text);
    if (!readLine(text))
      return false;
  }
  return !f.bad();
}

} // namespace text_palette_file
} // namespace doc::file
