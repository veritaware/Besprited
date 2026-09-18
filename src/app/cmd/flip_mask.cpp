// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/cmd/flip_mask.h"

#include "app/document.h"
#include "doc/algorithm/flip_image.h"
#include "doc/mask.h"

namespace app::cmd
{

using namespace doc;

FlipMask::FlipMask(const Document* doc, const algorithm::FlipType flipType)
  : WithDocument(doc)
  , m_flipType(flipType)
{
}

void FlipMask::onExecute()
{
  swap();
}

void FlipMask::onUndo()
{
  swap();
}

void FlipMask::swap() const
{
  const Document* document = this->document();
  Mask* mask = document->mask();

  ASSERT(mask->bitmap());
  if (!mask->bitmap())
    return;

  mask->freeze();
  doc::algorithm::flip_image(mask->bitmap(), mask->bitmap()->bounds(),
                             m_flipType);
  mask->unfreeze();
}

} // namespace app::cmd
