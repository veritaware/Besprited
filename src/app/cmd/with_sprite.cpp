// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/cmd/with_sprite.h"

#include "doc/sprite.h"

namespace app::cmd
{

using namespace doc;

WithSprite::WithSprite(const Sprite* sprite)
  : m_spriteId(sprite->id())
{
}

Sprite* WithSprite::sprite() const
{
  return get<Sprite>(m_spriteId);
}

} // namespace app::cmd
