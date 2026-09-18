// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/cmd/remap_colors.h"

#include <utility>

#include "doc/cel.h"
#include "doc/cels_range.h"
#include "doc/image.h"
#include "doc/remap.h"
#include "doc/sprite.h"

namespace app::cmd
{

using namespace doc;

RemapColors::RemapColors(const Sprite* sprite, Remap remap)
  : WithSprite(sprite)
  , m_remap(std::move(remap))
{
}

void RemapColors::onExecute()
{
  if (Sprite* spr = sprite(); spr->pixelFormat() == IMAGE_INDEXED)
  {
    spr->remapImages(0, spr->lastFrame(), m_remap);
    incrementVersions(spr);
  }
}

void RemapColors::onUndo()
{
  if (Sprite* spr = this->sprite(); spr->pixelFormat() == IMAGE_INDEXED)
  {
    spr->remapImages(0, spr->lastFrame(), m_remap.invert());
    incrementVersions(spr);
  }
}

void RemapColors::incrementVersions(const Sprite* spr)
{
  for (const auto cel : spr->uniqueCels())
    cel->image()->incrementVersion();
}

} // namespace app::cmd
