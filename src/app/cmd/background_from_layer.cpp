// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/cmd/background_from_layer.h"

#include "app/cmd/add_cel.h"
#include "app/cmd/configure_background.h"
#include "app/cmd/copy_rect.h"
#include "app/cmd/replace_image.h"
#include "app/cmd/set_cel_opacity.h"
#include "app/cmd/set_cel_position.h"
#include "app/document.h"
#include "doc/cel.h"
#include "doc/image.h"
#include "doc/layer.h"
#include "doc/primitives.h"
#include "doc/sprite.h"
#include "render/render.h"

namespace app::cmd
{

BackgroundFromLayer::BackgroundFromLayer(const Layer* layer)
  : WithLayer(layer)
{
  ASSERT(layer);
  // cppcheck-suppress nullPointerRedundantCheck
  ASSERT(layer->isVisible());
  ASSERT(layer->isEditable());
  ASSERT(layer->sprite() != nullptr);
  // cppcheck-suppress nullPointerRedundantCheck
  ASSERT(layer->sprite()->backgroundLayer() == nullptr);
}

void BackgroundFromLayer::onExecute()
{
  Layer* layer = this->layer();
  Sprite* sprite = layer->sprite();
  const auto* doc = dynamic_cast<Document*>(sprite->document());
  const color_t bg_color = doc->bgColor();

  // create a temporary image to draw each frame of the new
  // `Background' layer
  const ImageRef bg_image(
      Image::create(sprite->pixelFormat(), sprite->width(), sprite->height()));

  CelList cels;
  layer->getCels(cels);
  for (const auto& cel : cels)
  {
    // get the image from the sprite's stock of images
    const Image* cel_image = cel->image();
    ASSERT(cel_image);
    if (!cel_image)
      continue;

    clear_image(bg_image.get(), bg_color);
    render::composite_image(bg_image.get(), cel_image,
                            sprite->palette(cel->frame()), cel->x(), cel->y(),
                            MID(0, cel->opacity(), 255),
                            dynamic_cast<LayerImage*>(layer)->blendMode());

    // now we have to copy the new image (bg_image) to the cel...
    executeAndAdd(new SetCelPosition(cel, 0, 0));

    // change opacity to 255
    if (cel->opacity() < 255)
      executeAndAdd(new SetCelOpacity(cel, 255));

    // same size of cel-image and bg-image
    if (bg_image->width() == cel_image->width() &&
        bg_image->height() == cel_image->height())
    {
      executeAndAdd(new CopyRect(cel_image, bg_image.get(),
                                 gfx::Clip(0, 0, cel_image->bounds())));
    }
    else
    {
      const ImageRef bg_image2(Image::createCopy(bg_image.get()));
      executeAndAdd(new ReplaceImage(sprite, cel->imageRef(), bg_image2));
    }
  }

  // Fill all empty cels with a flat-image filled with bg_color
  for (frame_t frame(0); frame < sprite->totalFrames(); ++frame)
  {
    auto cel = layer->cel(frame);
    if (!cel)
    {
      const ImageRef cel_image(Image::create(
          sprite->pixelFormat(), sprite->width(), sprite->height()));
      clear_image(cel_image.get(), bg_color);

      // Create the new cel and add it to the new background layer
      cel = std::make_shared<Cel>(frame, cel_image);
      executeAndAdd(new AddCel(layer, cel));
    }
  }

  executeAndAdd(new ConfigureBackground(layer));
}

} // namespace app::cmd
