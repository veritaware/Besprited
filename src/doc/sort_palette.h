// Document Library
// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

namespace doc
{

class Remap;
class Palette;

enum class SortPaletteBy
{
  RED,
  GREEN,
  BLUE,
  ALPHA,
  HUE,
  SATURATION,
  VALUE,
  LIGHTNESS,
  LUMA,
};

// Creates a Remap to sort the palette. It doesn't apply the remap.
Remap sort_palette(Palette* palette, SortPaletteBy channel, bool ascending);

} // namespace doc
