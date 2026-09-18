// Document Library
// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <string>

namespace doc
{

void write_string(std::ostream& os, const std::string& str);
std::string read_string(std::istream& is);

} // namespace doc
