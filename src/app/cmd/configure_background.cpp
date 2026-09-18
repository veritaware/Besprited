// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/cmd/configure_background.h"

#include "app/cmd/move_layer.h"
#include "app/cmd/set_layer_flags.h"
#include "app/cmd/set_layer_name.h"
#include "app/cmd/set_layer_opacity.h"

namespace app::cmd
{

ConfigureBackground::ConfigureBackground(Layer* layer)
{
  // Add "Background" and "LockMove" flags
  const auto newFlags = static_cast<LayerFlags>(
      static_cast<int>(layer->flags()) |
      static_cast<int>(LayerFlags::BackgroundLayerFlags));

  add(new SetLayerFlags(layer, newFlags));
  add(new SetLayerName(layer, "Background"));

  if (layer->isImage() && dynamic_cast<LayerImage*>(layer)->opacity() < 255)
  {
    add(new SetLayerOpacity(dynamic_cast<LayerImage*>(layer), 255));
  }

  add(new MoveLayer(layer, nullptr));
}

} // namespace app::cmd
