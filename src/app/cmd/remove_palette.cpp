// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/cmd/remove_palette.h"

namespace app::cmd
{

using namespace doc;

RemovePalette::RemovePalette(const Sprite* sprite, const Palette& pal)
  : AddPalette(sprite, pal)
{
}

void RemovePalette::onExecute()
{
  AddPalette::onUndo();
}

void RemovePalette::onUndo()
{
  AddPalette::onRedo();
}

} // namespace app::cmd
