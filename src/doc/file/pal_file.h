// Document Library
// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

namespace doc
{

class Palette;

namespace file
{

std::shared_ptr<Palette> load_pal_file(const char* filename);
bool save_pal_file(const Palette& pal, const char* filename);

} // namespace file
} // namespace doc
