// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/cmd/set_mask_position.h"

#include "app/document.h"
#include "doc/mask.h"

namespace app::cmd
{

SetMaskPosition::SetMaskPosition(const Document* doc, const gfx::Point& pos)
  : WithDocument(doc)
  , m_oldPosition(doc->mask()->bounds().origin())
  , m_newPosition(pos)
{
}

void SetMaskPosition::onExecute()
{
  setMaskPosition(m_newPosition);
}

void SetMaskPosition::onUndo()
{
  setMaskPosition(m_oldPosition);
}

void SetMaskPosition::setMaskPosition(const gfx::Point& pos) const
{
  Document* doc = document();
  doc->mask()->setOrigin(pos.x, pos.y);
  doc->resetTransformation();
}

} // namespace app::cmd
