// Besprited | Copyright (C) 2026 Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "doc/color.h"
#include "filters/filter_manager.h"
#include "filters/target.h"

#include <cstdint>

// Shared per-row driver for the pointwise filters (invert, replace-color,
// color-curve, ...), which otherwise all hand-write the same skeleton: cast
// src/dst addresses, walk getWidth() pixels honoring skipPixel(), decompose
// the pixel into channels, apply the filter's per-channel logic only to
// channels named in the Target mask, recompose, write. See issue #226.
//
// This intentionally does NOT try to unify the neighborhood filters
// (median, convolution matrix) - their outer walk still fits for_each_pixel,
// but their per-channel accumulation logic doesn't reduce to a single
// map_*_channels() call the way a pointwise filter's does.
namespace filters::detail
{

// Walks every pixel of the row FilterManager is currently pointing at,
// honoring skipPixel() exactly like every filter's hand-written loop did:
// a skipped pixel's dst is left untouched and both pointers still advance.
// `op(rawValue, x)` returns the pixel's replacement value.
template <class PixelT, class Op>
void for_each_pixel(FilterManager& m, Op&& op)
{
  const auto* src = static_cast<const PixelT*>(m.getSourceAddress());
  auto* dst = static_cast<PixelT*>(m.getDestinationAddress());
  const int w = m.getWidth();

  for (int x = 0; x < w; ++x)
  {
    if (m.skipPixel())
    {
      ++src;
      ++dst;
      continue;
    }

    *dst = static_cast<PixelT>(op(static_cast<int>(*src), x));
    ++src;
    ++dst;
  }
}

// Decomposes an RGBA pixel, calls fn(currentValue, channelBit) for each of
// R/G/B/A named in `target`, and recomposes. Channels not in `target` are
// passed through unchanged.
template <class Fn>
doc::color_t map_rgba_channels(doc::color_t c, Target target, Fn&& fn)
{
  int r = doc::rgba_getr(c);
  int g = doc::rgba_getg(c);
  int b = doc::rgba_getb(c);
  int a = doc::rgba_geta(c);

  if (target & TARGET_RED_CHANNEL)
    r = fn(r, TARGET_RED_CHANNEL);
  if (target & TARGET_GREEN_CHANNEL)
    g = fn(g, TARGET_GREEN_CHANNEL);
  if (target & TARGET_BLUE_CHANNEL)
    b = fn(b, TARGET_BLUE_CHANNEL);
  if (target & TARGET_ALPHA_CHANNEL)
    a = fn(a, TARGET_ALPHA_CHANNEL);

  return doc::rgba(r, g, b, a);
}

// Same idea for a grayscale+alpha pixel.
template <class Fn>
uint16_t map_gray_channels(uint16_t c, Target target, Fn&& fn)
{
  int v = doc::graya_getv(c);
  int a = doc::graya_geta(c);

  if (target & TARGET_GRAY_CHANNEL)
    v = fn(v, TARGET_GRAY_CHANNEL);
  if (target & TARGET_ALPHA_CHANNEL)
    a = fn(a, TARGET_ALPHA_CHANNEL);

  return doc::graya(v, a);
}

} // namespace filters::detail
