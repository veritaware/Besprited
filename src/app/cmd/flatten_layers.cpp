// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/cmd/flatten_layers.h"

#include "app/cmd/add_layer.h"
#include "app/cmd/set_layer_name.h"
#include "app/cmd/configure_background.h"
#include "app/cmd/copy_rect.h"
#include "app/cmd/remove_layer.h"
#include "app/cmd/unlink_cel.h"
#include "app/document.h"
#include "doc/cel.h"
#include "doc/layer.h"
#include "doc/primitives.h"
#include "doc/sprite.h"
#include "render/render.h"

namespace app::cmd
{

FlattenLayers::FlattenLayers(const Sprite* sprite)
  : WithSprite(sprite)
{
}

void FlattenLayers::onExecute()
{
  Sprite* sprite = this->sprite();
  const auto* doc = dynamic_cast<Document*>(sprite->document());

  // Create a temporary image.
  const ImageRef image(
      Image::create(sprite->pixelFormat(), sprite->width(), sprite->height()));

  // The layer onto which everything will be flattened.
  LayerImage* flatLayer = sprite->backgroundLayer();
  color_t bgcolor; // The background color to use for flatLayer.

  if (flatLayer && flatLayer->isVisible())
  {
    // There exists a visible background layer, so we will flatten onto that.
    bgcolor = doc->bgColor(flatLayer);
  }
  else
  {
    // Create a new transparent layer to flatten everything onto.
    flatLayer = new LayerImage(sprite);
    ASSERT(flatLayer->isVisible());
    executeAndAdd(new AddLayer(sprite->folder(), flatLayer, nullptr));
    executeAndAdd(new SetLayerName(flatLayer, "Flattened"));
    bgcolor = sprite->transparentColor();
  }

  render::Render render;
  render.setBgType(render::BgType::NONE);

  // Copy all frames to the background.
  for (frame_t frame(0); frame < sprite->totalFrames(); ++frame)
  {
    // Clear the image and render this frame.
    clear_image(image.get(), bgcolor);
    render.renderSprite(image.get(), sprite, frame);

    // TODO Keep cel links when possible

    ImageRef cel_image;
    auto cel = flatLayer->cel(frame);
    if (cel)
    {
      if (cel->links())
        executeAndAdd(new UnlinkCel(cel));

      cel_image = cel->imageRef();
      ASSERT(cel_image);

      executeAndAdd(new CopyRect(cel_image.get(), image.get(),
                                 gfx::Clip(0, 0, image->bounds())));
    }
    else
    {
      cel_image.reset(Image::createCopy(image.get()));
      cel = std::make_shared<Cel>(frame, cel_image);
      flatLayer->addCel(cel);
    }
  }

  // Delete old layers.
  for (const LayerList layers = sprite->folder()->getLayersList();
       Layer* layer : layers)
    if (layer != flatLayer)
      executeAndAdd(new RemoveLayer(layer));
}

} // namespace app::cmd
