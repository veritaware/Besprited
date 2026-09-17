// UI Library
// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include "gfx/region.h"

namespace ui
{

class Manager;

void move_region(Manager* manager, const gfx::Region& region, int dx, int dy);

} // namespace ui
