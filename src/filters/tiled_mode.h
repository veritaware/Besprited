// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

namespace filters
{

enum class [[clang::flag_enum]] TiledMode
{
  NONE = 0,
  X_AXIS = 1,
  Y_AXIS = 2,
  BOTH = 3,
};

} // namespace filters
