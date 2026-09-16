// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/cmd/reselect_mask.h"

#include "app/cmd/set_mask.h"
#include "app/document.h"
#include "doc/mask.h"

namespace app::cmd
{

ReselectMask::ReselectMask(const Document* doc)
  : WithDocument(doc)
{
}

void ReselectMask::onExecute()
{
  Document* doc = document();

  if (m_oldMask)
  {
    doc->setMask(m_oldMask.get());
    m_oldMask.reset();
  }

  doc->setMaskVisible(true);
}

void ReselectMask::onUndo()
{
  Document* doc = document();

  m_oldMask.reset(doc->isMaskVisible() ? new Mask(*doc->mask()) : nullptr);

  doc->setMaskVisible(false);
}

size_t ReselectMask::onMemSize() const
{
  return sizeof(*this) + (m_oldMask ? m_oldMask->getMemSize() : 0);
}

} // namespace app::cmd
