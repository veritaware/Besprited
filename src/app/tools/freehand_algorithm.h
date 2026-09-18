// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include <cstdint>

namespace app::tools
{

enum class FreehandAlgorithm : std::uint8_t
{
  DEFAULT = 0,
  REGULAR = 0,
  PIXEL_PERFECT = 1,
  DOTS = 2,
};

} // namespace app::tools
