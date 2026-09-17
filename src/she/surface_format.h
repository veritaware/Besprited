// SHE Library
// Aseprite  | Copyright (C) 2012-2013 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <cstdint>

namespace she
{

enum SurfaceFormat : std::uint8_t
{
  kRgbaSurfaceFormat,
};

struct SurfaceFormatData
{
  SurfaceFormat format;
  uint32_t bitsPerPixel;
  uint32_t redShift;
  uint32_t greenShift;
  uint32_t blueShift;
  uint32_t alphaShift;
  uint32_t redMask;
  uint32_t greenMask;
  uint32_t blueMask;
  uint32_t alphaMask;
};

} // namespace she
