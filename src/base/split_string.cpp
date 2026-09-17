// Base Library
// Aseprite  | Copyright (C) 2001-2013, 2015 David Capello
// Besprited | Copyright (C) 2026            Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "base/split_string.h"

#include <algorithm>

namespace
{

struct is_separator
{
  const std::string* separators;

  is_separator(const std::string* seps)
    : separators(seps)
  {
  }

  bool operator()(std::string::value_type chr)
  {
    for (const char it : *separators)
    {
      if (chr == it)
        return true;
    }
    return false;
  }
};

} // namespace

void base::split_string(const std::string& string,
                        std::vector<std::string>& parts,
                        const std::string& separators)
{
  const std::size_t elements = 1 + std::count_if(string.begin(), string.end(),
                                                 is_separator(&separators));
  parts.reserve(elements);

  std::size_t beg = 0, end;
  while (true)
  {
    end = string.find_first_of(separators, beg);
    if (end != std::string::npos)
    {
      parts.push_back(string.substr(beg, end - beg));
      beg = end + 1;
    }
    else
    {
      parts.push_back(string.substr(beg));
      break;
    }
  }
}
