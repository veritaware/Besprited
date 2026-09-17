// Document Library
// Aseprite  | Copyright (C) 2001-2016 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include "gfx/fwd.h"
#include "doc/algorithm/flip_type.h"
#include "doc/color.h"

namespace doc
{
class Image;

namespace algorithm
{

bool shrink_bounds(const Image* image, const gfx::Rect& start_bounds,
                   gfx::Rect& bounds, color_t refpixel);

bool shrink_bounds(const Image* image, gfx::Rect& bounds, color_t refpixel);

bool shrink_bounds2(const Image* a, const Image* b,
                    const gfx::Rect& start_bounds, gfx::Rect& bounds);

} // namespace algorithm
} // namespace doc
