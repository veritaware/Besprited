// Render Library
// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <cstdint>

namespace render
{

enum class ExtraType : std::uint8_t
{
  NONE,

  // The extra cel indicates a "patch" for the current layer/frame
  // given in Render::setExtraImage()
  PATCH,

  // The extra cel indicates an extra composition for the current
  // layer/frame.
  COMPOSITE,
};

} // namespace render
