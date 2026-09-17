// Aseprite  | Copyright (C) 2001-2016 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include <cstdint>

namespace app::tools
{

// The trace policy indicates how pixels are updated between the
// source image (ToolLoop::getSrcImage) and destionation image
// (ToolLoop::getDstImage) in the whole ToolLoopManager life-time.
// Basically it says if we should accumulate the intertwined
// drawed points, or kept the last ones, or overlap/composite
// them, etc.
enum class TracePolicy : std::uint8_t
{

  // All pixels are accumulated in the destination image. Used by
  // freehand like tools.
  Accumulate,

  // It's like accumulate, but the last modified area in the
  // destination is invalidated and redraw from the source image +
  // tool trace. It's used by pixel-perfect freehand algorithm
  // (because last modified pixels can differ).
  AccumulateUpdateLast,

  // Only the last trace is used. It means that on each ToolLoop
  // step, the destination image is completely invalidated and
  // restored from the source image. Used by
  // line/rectangle/ellipse-like tools.
  Last,

  // Like accumulate, but the destination is copied to the source
  // on each ToolLoop step, so the tool overlaps its own effect.
  // Used by jumble and spray.
  Overlap,

};

} // namespace app::tools
