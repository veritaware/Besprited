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

namespace
{

std::string trimmed(const std::string& s)
{
  std::string out;
  base::trim_string(s, out);
  return out;
}

// Single-line, tab-free text safe to write into a .gpl file.
std::string sanitized(std::string s)
{
  for (char& c : s)
    if (c == '\n' || c == '\r' || c == '\t')
      c = ' ';
  return s;
}

// Case-insensitive prefix check; on match stores the trimmed remainder in "rest".
bool strip_prefix(const std::string& line, const char* prefix, std::string& rest)
{
  const size_t n = std::char_traits<char>::length(prefix);
  if (line.size() < n)
    return false;
  for (size_t i = 0; i < n; ++i)
    if (std::tolower((unsigned char)line[i]) != std::tolower((unsigned char)prefix[i]))
      return false;
  rest = trimmed(line.substr(n));
  return true;
}

void read_header_comment(Palette& pal, const std::string& comment)
{
  const std::string text = trimmed(comment.substr(1));
  std::string rest;
  if (strip_prefix(text, "Palette Name:", rest))
  {
    if (!rest.empty())
      pal.setName(rest);
  }
  else if (strip_prefix(text, "Author:", rest) || strip_prefix(text, "by ", rest))
  {
    if (!rest.empty() && pal.author().empty())
      pal.setAuthor(rest);
  }
}

} // namespace

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

    if (line.empty())
      continue;

    // Comments, which may carry "Palette Name:" / "Author:" / "by" metadata
    if (line[0] == '#')
    {
      if (pal->size() == 0) // metadata lives in the header only
        read_header_comment(*pal, line);
      continue;
    }

    // "Name: XYZ" property
    if (!std::isdigit(line[0]))
    {
      if (line.compare(0, 5, "Name:") == 0 && pal->name().empty())
        pal->setName(trimmed(line.substr(5)));
      continue;
    }

    int r, g, b;
    std::istringstream lineIn(line);
    lineIn >> r >> g >> b;

    if (lineIn.fail())
      continue;

    std::string name;
    std::getline(lineIn, name);
    name = trimmed(name);

    pal->addEntry(rgba(r, g, b, 255));
    if (name != "Untitled")
      pal->setEntryName(pal->size() - 1, name);
  }

  return pal;
}

bool save_gpl_file(const Palette& pal, const char* filename)
{
  std::ofstream f(FSTREAM_PATH(filename));
  if (f.bad())
    return false;

  f << "GIMP Palette\n";
  if (!pal.name().empty())
    f << "#Palette Name: " << sanitized(pal.name()) << "\n";
  if (!pal.author().empty())
    f << "#Author: " << sanitized(pal.author()) << "\n";
  f << "#Colors: " << pal.size() << "\n";

  for (int i = 0; i < pal.size(); ++i)
  {
    const uint32_t col = pal.getEntry(i);
    f << std::setfill(' ') << std::setw(3) << (static_cast<int>(rgba_getr(col))) << " " << std::setfill(' ')
      << std::setw(3) << (static_cast<int>(rgba_getg(col))) << " " << std::setfill(' ') << std::setw(3)
      << (static_cast<int>(rgba_getb(col))) << "\t"
      << (pal.entryName(i).empty() ? std::string("Untitled") : sanitized(pal.entryName(i))) << "\n";
  }

  return true;
}

} // namespace doc::file
