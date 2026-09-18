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

enum BrushType
{
  kCircleBrushType = 0,
  kSquareBrushType = 1,
  kLineBrushType = 2,
  kImageBrushType = 3,

  kFirstBrushType = kCircleBrushType,
  kLastBrushType = kImageBrushType,
};

std::string brush_type_to_string_id(BrushType brushType);
BrushType string_id_to_brush_type(const std::string& s);

} // namespace doc
