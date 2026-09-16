// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/cmd/with_layer.h"

#include "doc/layer.h"

namespace app::cmd
{

using namespace doc;

WithLayer::WithLayer(const Layer* layer)
  : m_layerId(layer ? layer->id() : 0)
{
}

Layer* WithLayer::layer() const
{
  if (m_layerId)
    return get<Layer>(m_layerId);
  return nullptr;
}

} // namespace app::cmd
