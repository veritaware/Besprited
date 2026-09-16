// Aseprite  | Copyright (C) 2001-2016 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/cmd/add_palette.h"

#include "doc/sprite.h"
#include "doc/palette.h"
#include "doc/palette_io.h"

namespace app::cmd
{

using namespace doc;

AddPalette::AddPalette(const Sprite* sprite, const Palette& pal)
  : WithSprite(sprite)
  , m_size(0)
  , m_frame(pal.frame())
{
  write_palette(m_stream, pal);
  m_size = static_cast<size_t>(m_stream.tellp());
}

void AddPalette::onExecute()
{
  m_stream.seekp(0);

  Sprite* sprite = this->sprite();
  const auto pal = read_palette(m_stream);

  sprite->setPalette(*pal, true);
  sprite->incrementVersion();
}

void AddPalette::onUndo()
{
  Sprite* sprite = this->sprite();

  sprite->deletePalette(m_frame);
  sprite->incrementVersion();
}

} // namespace app::cmd
