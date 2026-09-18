// Document Library
// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <iosfwd>

namespace doc
{

class Cel;
class SubObjectsIO;

void write_cel(std::ostream& os, const Cel* cel);
Cel* read_cel(std::istream& is, SubObjectsIO* subObjects, bool setId = true);

} // namespace doc
