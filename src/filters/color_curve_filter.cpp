// Aseprite  | Copyright (C) 2001-2016 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "filters/color_curve_filter.h"

#include "base/base.h"
#include "filters/color_curve.h"
#include "filters/filter_indexed_data.h"
#include "filters/filter_manager.h"
#include "doc/image.h"
#include "doc/palette.h"
#include "doc/rgbmap.h"
#include "doc/sprite.h"

#include <vector>

namespace filters
{

using namespace doc;

ColorCurveFilter::ColorCurveFilter()
  : m_curve(nullptr)
  , m_cmap(256)
{
}

void ColorCurveFilter::setCurve(ColorCurve* curve)
{
  ASSERT(curve != nullptr);

  m_curve = curve;

  // Generate the color convertion map
  // cppcheck-suppress nullPointerRedundantCheck
  m_curve->getValues(0, 255, m_cmap);
  for (int c = 0; c < 256; c++)
    m_cmap[c] = MID(0, m_cmap[c], 255);
}

const char* ColorCurveFilter::getName()
{
  return "Color Curve";
}

void ColorCurveFilter::applyToRgba(FilterManager* filterMgr)
{
  const auto* src_address =
      static_cast<const uint32_t*>(filterMgr->getSourceAddress());
  auto* dst_address =
      static_cast<uint32_t*>(filterMgr->getDestinationAddress());
  const int w = filterMgr->getWidth();
  const Target target = filterMgr->getTarget();
  int x, c, r, g, b, a;

  for (x = 0; x < w; x++)
  {
    if (filterMgr->skipPixel())
    {
      ++src_address;
      ++dst_address;
      continue;
    }

    c = static_cast<int>(*(src_address++));

    r = rgba_getr(c);
    g = rgba_getg(c);
    b = rgba_getb(c);
    a = rgba_geta(c);

    if (target & TARGET_RED_CHANNEL)
      r = m_cmap[r];
    if (target & TARGET_GREEN_CHANNEL)
      g = m_cmap[g];
    if (target & TARGET_BLUE_CHANNEL)
      b = m_cmap[b];
    if (target & TARGET_ALPHA_CHANNEL)
      a = m_cmap[a];

    *(dst_address++) = rgba(r, g, b, a);
  }
}

void ColorCurveFilter::applyToGrayscale(FilterManager* filterMgr)
{
  const auto* src_address =
      static_cast<const uint16_t*>(filterMgr->getSourceAddress());
  auto* dst_address =
      static_cast<uint16_t*>(filterMgr->getDestinationAddress());
  const int w = filterMgr->getWidth();
  const Target target = filterMgr->getTarget();
  int x, c, k, a;

  for (x = 0; x < w; x++)
  {
    if (filterMgr->skipPixel())
    {
      ++src_address;
      ++dst_address;
      continue;
    }

    c = static_cast<int>(*(src_address++));

    k = graya_getv(c);
    a = graya_geta(c);

    if (target & TARGET_GRAY_CHANNEL)
      k = m_cmap[k];
    if (target & TARGET_ALPHA_CHANNEL)
      a = m_cmap[a];

    *(dst_address++) = graya(k, a);
  }
}

void ColorCurveFilter::applyToIndexed(FilterManager* filterMgr)
{
  const auto* src_address =
      static_cast<const uint8_t*>(filterMgr->getSourceAddress());
  auto* dst_address = static_cast<uint8_t*>(filterMgr->getDestinationAddress());
  const int w = filterMgr->getWidth();
  const Target target = filterMgr->getTarget();
  const Palette* pal = filterMgr->getIndexedData()->getPalette();
  const RgbMap* rgbmap = filterMgr->getIndexedData()->getRgbMap();
  int x, c, r, g, b, a;

  for (x = 0; x < w; x++)
  {
    if (filterMgr->skipPixel())
    {
      ++src_address;
      ++dst_address;
      continue;
    }

    c = static_cast<int>(*(src_address++));

    if (target & TARGET_INDEX_CHANNEL)
    {
      c = m_cmap[c];
    }
    else
    {
      c = pal->getEntry(c);
      r = rgba_getr(c);
      g = rgba_getg(c);
      b = rgba_getb(c);
      a = rgba_geta(c);

      if (target & TARGET_RED_CHANNEL)
        r = m_cmap[r];
      if (target & TARGET_GREEN_CHANNEL)
        g = m_cmap[g];
      if (target & TARGET_BLUE_CHANNEL)
        b = m_cmap[b];
      if (target & TARGET_ALPHA_CHANNEL)
        a = m_cmap[a];

      c = rgbmap->mapColor(r, g, b, a);
    }

    *(dst_address++) = MID(0, c, pal->size() - 1);
  }
}

} // namespace filters
