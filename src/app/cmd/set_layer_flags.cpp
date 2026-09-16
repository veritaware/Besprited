// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/cmd/set_layer_flags.h"

namespace app::cmd
{

SetLayerFlags::SetLayerFlags(const Layer* layer, const LayerFlags flags)
  : WithLayer(layer)
  , m_oldFlags(layer->flags())
  , m_newFlags(flags)
{
}

void SetLayerFlags::onExecute()
{
  layer()->setFlags(m_newFlags);
  layer()->incrementVersion();
}

void SetLayerFlags::onUndo()
{
  layer()->setFlags(m_oldFlags);
  layer()->incrementVersion();
}

} // namespace app::cmd
