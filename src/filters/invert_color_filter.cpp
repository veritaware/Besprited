// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "filters/invert_color_filter.h"

#include "filters/filter_indexed_data.h"
#include "filters/filter_manager.h"
#include "doc/image.h"
#include "doc/palette.h"
#include "doc/rgbmap.h"

namespace filters
{

using namespace doc;

const char* InvertColorFilter::getName()
{
  return "Invert Color";
}

void InvertColorFilter::applyToRgba(FilterManager* filterMgr)
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
      r ^= 0xff;
    if (target & TARGET_GREEN_CHANNEL)
      g ^= 0xff;
    if (target & TARGET_BLUE_CHANNEL)
      b ^= 0xff;
    if (target & TARGET_ALPHA_CHANNEL)
      a ^= 0xff;

    *(dst_address++) = rgba(r, g, b, a);
  }
}

void InvertColorFilter::applyToGrayscale(FilterManager* filterMgr)
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
      k ^= 0xff;
    if (target & TARGET_ALPHA_CHANNEL)
      a ^= 0xff;

    *(dst_address++) = graya(k, a);
  }
}

void InvertColorFilter::applyToIndexed(FilterManager* filterMgr)
{
  const auto* src_address =
      static_cast<const uint8_t*>(filterMgr->getSourceAddress());
  auto* dst_address = static_cast<uint8_t*>(filterMgr->getDestinationAddress());
  const Palette* pal = filterMgr->getIndexedData()->getPalette();
  const RgbMap* rgbmap = filterMgr->getIndexedData()->getRgbMap();
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

    if (target & TARGET_INDEX_CHANNEL)
      c ^= 0xff;
    else
    {
      c = pal->getEntry(c);
      r = rgba_getr(c);
      g = rgba_getg(c);
      b = rgba_getb(c);
      a = rgba_geta(c);

      if (target & TARGET_RED_CHANNEL)
        r ^= 0xff;
      if (target & TARGET_GREEN_CHANNEL)
        g ^= 0xff;
      if (target & TARGET_BLUE_CHANNEL)
        b ^= 0xff;
      if (target & TARGET_ALPHA_CHANNEL)
        a ^= 0xff;

      c = rgbmap->mapColor(r, g, b, a);
    }

    *(dst_address++) = c;
  }
}

} // namespace filters
