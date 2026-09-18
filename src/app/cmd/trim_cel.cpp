// Aseprite  | Copyright (C) 2016 David Capello
// Besprited | Copyright (C) 2026 Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/cmd/trim_cel.h"

#include "app/cmd/crop_cel.h"
#include "app/cmd/remove_cel.h"
#include "doc/algorithm/shrink_bounds.h"
#include "doc/cel.h"

namespace app::cmd
{

using namespace doc;

TrimCel::TrimCel(const std::shared_ptr<Cel>& cel)
{
  if (gfx::Rect newBounds; algorithm::shrink_bounds(cel->image(), newBounds,
                                                    cel->image()->maskColor()))
  {
    newBounds.offset(cel->position());
    m_subCmd = new CropCel(cel, newBounds);
  }
  else
  {
    m_subCmd = new RemoveCel(cel);
  }
}

TrimCel::~TrimCel()
{
  delete m_subCmd;
}

void TrimCel::onExecute()
{
  m_subCmd->execute(context());
}

void TrimCel::onUndo()
{
  m_subCmd->undo();
}

void TrimCel::onRedo()
{
  m_subCmd->redo();
}

} // namespace app::cmd
