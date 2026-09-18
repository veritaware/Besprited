// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/cmd/set_layer_name.h"

#include <utility>

#include "doc/document.h"
#include "doc/document_event.h"
#include "doc/layer.h"
#include "doc/sprite.h"

namespace app::cmd
{

SetLayerName::SetLayerName(const Layer* layer, std::string name)
  : WithLayer(layer)
  , m_oldName(layer->name())
  , m_newName(std::move(name))
{
}

void SetLayerName::onExecute()
{
  layer()->setName(m_newName);
  layer()->incrementVersion();
}

void SetLayerName::onUndo()
{
  layer()->setName(m_oldName);
  layer()->incrementVersion();
}

void SetLayerName::onFireNotifications()
{
  Layer* layer = this->layer();
  Document* doc = layer->sprite()->document();
  DocumentEvent ev(doc);
  ev.sprite(layer->sprite());
  ev.layer(layer);
  doc->notifyObservers<DocumentEvent&>(&DocumentObserver::onLayerNameChange,
                                       ev);
}

} // namespace app::cmd
