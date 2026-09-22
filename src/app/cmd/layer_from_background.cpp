// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/cmd/layer_from_background.h"

#include "app/cmd/set_layer_flags.h"
#include "app/cmd/set_layer_name.h"
#include "doc/layer.h"
#include "doc/sprite.h"

namespace app::cmd
{

LayerFromBackground::LayerFromBackground(const Layer* layer)
{
  ASSERT(layer != nullptr);
  // cppcheck-suppress nullPointerRedundantCheck
  ASSERT(layer->isVisible());
  ASSERT(layer->isEditable());
  ASSERT(layer->isBackground());
  ASSERT(layer->sprite() != nullptr);
  // cppcheck-suppress nullPointerRedundantCheck
  ASSERT(layer->sprite()->backgroundLayer() != nullptr);

  // Remove "Background" and "LockMove" flags
  const auto newFlags = static_cast<LayerFlags>(
      static_cast<int>(layer->flags()) &
      ~static_cast<int>(LayerFlags::BackgroundLayerFlags));

  add(new SetLayerFlags(layer, newFlags));
  add(new SetLayerName(layer, "Layer 0"));
}

} // namespace app::cmd
