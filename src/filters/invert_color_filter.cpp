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
#include "filters/pixel_row.h"
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
  const Target target = filterMgr->getTarget();

  detail::for_each_pixel<uint32_t>(
      *filterMgr,
      [target](int c, int)
      {
        return detail::map_rgba_channels(
            static_cast<color_t>(c), target,
            [](int v, Target) { return v ^ 0xff; });
      });
}

void InvertColorFilter::applyToGrayscale(FilterManager* filterMgr)
{
  const Target target = filterMgr->getTarget();

  detail::for_each_pixel<uint16_t>(
      *filterMgr,
      [target](int c, int)
      {
        return detail::map_gray_channels(
            static_cast<uint16_t>(c), target,
            [](int v, Target) { return v ^ 0xff; });
      });
}

void InvertColorFilter::applyToIndexed(FilterManager* filterMgr)
{
  const Target target = filterMgr->getTarget();
  const Palette* pal = filterMgr->getIndexedData()->getPalette();
  const RgbMap* rgbmap = filterMgr->getIndexedData()->getRgbMap();

  detail::for_each_pixel<uint8_t>(
      *filterMgr,
      [target, pal, rgbmap](int c, int) -> int
      {
        if (target & TARGET_INDEX_CHANNEL)
          return c ^ 0xff;

        color_t rgbaColor = detail::map_rgba_channels(
            pal->getEntry(c), target, [](int v, Target) { return v ^ 0xff; });
        return rgbmap->mapColor(rgba_getr(rgbaColor), rgba_getg(rgbaColor),
                                rgba_getb(rgbaColor), rgba_geta(rgbaColor));
      });
}

} // namespace filters
