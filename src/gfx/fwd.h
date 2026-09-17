// Gfx Library
// Aseprite  | Copyright (C) 2001-2016 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

namespace gfx
{

template <typename T> class BorderT;
template <typename T> class PointT;
template <typename T> class RectT;
template <typename T> class SizeT;

using Border = BorderT<int>;
using Point = PointT<int>;
using Rect = RectT<int>;
using Size = SizeT<int>;

class Region;

} // namespace gfx
