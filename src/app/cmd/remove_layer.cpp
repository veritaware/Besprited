// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/cmd/remove_layer.h"

#include "doc/layer.h"

namespace app::cmd
{

using namespace doc;

RemoveLayer::RemoveLayer(const Layer* layer)
  : AddLayer(layer->parent(), layer, layer->getPrevious())
{
}

void RemoveLayer::onExecute()
{
  AddLayer::onUndo();
}

void RemoveLayer::onUndo()
{
  AddLayer::onRedo();
}

void RemoveLayer::onRedo()
{
  AddLayer::onUndo();
}

} // namespace app::cmd
