// Document Library
// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "doc/layer_io.h"

#include "base/serialization.h"
#include "doc/cel.h"
#include "doc/cel_data.h"
#include "doc/cel_data_io.h"
#include "doc/cel_io.h"
#include "doc/image_io.h"
#include "doc/layer.h"
#include "doc/layer_io.h"
#include "doc/sprite.h"
#include "doc/string_io.h"
#include "doc/subobjects_io.h"
#include "doc/user_data_io.h"

#include <iostream>
#include <memory>
#include <vector>

namespace doc
{

using namespace base::serialization;
using namespace base::serialization::little_endian;

// Serialized Layer data:

void write_layer(std::ostream& os, const Layer* layer)
{
  write32(os, layer->id());
  write_string(os, layer->name());

  write32(os, static_cast<int>(layer->flags())); // Flags
  write16(os, static_cast<int>(layer->type()));  // Type

  switch (layer->type())
  {

  case ObjectType::LayerImage:
  {
    const auto* imgLayer = static_cast<const LayerImage*>(layer);
    CelConstIterator it, begin = imgLayer->getCelBegin();
    auto end = imgLayer->getCelEnd();

    // Blend mode & opacity
    write16(os, static_cast<int>(imgLayer->blendMode()));
    write8(os, imgLayer->opacity());

    // Images
    int images = 0;
    int celdatas = 0;
    for (it = begin; it != end; ++it)
    {
      auto cel = *it;
      if (!cel->link())
      {
        ++images;
        ++celdatas;
      }
    }

    write16(os, images);
    for (it = begin; it != end; ++it)
    {
      auto cel = *it;
      if (!cel->link())
        write_image(os, cel->image());
    }

    write16(os, celdatas);
    for (it = begin; it != end; ++it)
    {
      auto cel = *it;
      if (!cel->link())
        write_celdata(os, cel->dataRef().get());
    }

    // Cels
    write16(os, imgLayer->getCelsCount());
    for (it = begin; it != end; ++it)
    {
      write_cel(os, it->get());
    }
    break;
  }

  case ObjectType::LayerFolder:
  {
    auto it = static_cast<const LayerFolder*>(layer)->getLayerBegin();
    auto end = static_cast<const LayerFolder*>(layer)->getLayerEnd();

    // Number of sub-layers
    write16(os, static_cast<const LayerFolder*>(layer)->getLayersCount());

    for (; it != end; ++it)
      write_layer(os, *it);
    break;
  }
  }

  write_user_data(os, layer->userData());
}

Layer* read_layer(std::istream& is, SubObjectsFromSprite* subObjects)
{
  const ObjectId id = read32(is);
  const std::string name = read_string(is);
  const uint32_t flags = read32(is);      // Flags
  const uint16_t layer_type = read16(is); // Type
  std::unique_ptr<Layer> layer;

  switch (static_cast<ObjectType>(layer_type))
  {

  case ObjectType::LayerImage:
  {
    auto* imgLayer = new LayerImage(subObjects->sprite());

    // Create layer
    layer.reset(imgLayer);

    // Blend mode & opacity
    imgLayer->setBlendMode(static_cast<BlendMode>(read16(is)));
    imgLayer->setOpacity(read8(is));

    // Read images
    const int images = read16(is); // Number of images
    for (int c = 0; c < images; ++c)
    {
      const ImageRef image(read_image(is));
      subObjects->addImageRef(image);
    }

    // Read celdatas
    const int celdatas = read16(is);
    for (int c = 0; c < celdatas; ++c)
    {
      const CelDataRef celdata(read_celdata(is, subObjects));
      subObjects->addCelDataRef(celdata);
    }

    // Read cels
    const int cels = read16(is); // Number of cels
    for (int c = 0; c < cels; ++c)
    {
      // Read the cel
      auto cel = std::shared_ptr<Cel>(read_cel(is, subObjects));

      // Add the cel in the layer
      imgLayer->addCel(cel);
    }
    break;
  }

  case ObjectType::LayerFolder:
  {
    // Create the layer set
    layer = std::make_unique<LayerFolder>(subObjects->sprite());

    // Number of sub-layers
    const int layers = read16(is);
    for (int c = 0; c < layers; c++)
    {
      Layer* child = read_layer(is, subObjects);
      if (child)
        static_cast<LayerFolder*>(layer.get())->addLayer(child);
      else
        break;
    }
    break;
  }

  default:
    throw InvalidLayerType("Invalid layer type found in stream");
  }

  const UserData userData = read_user_data(is);

  if (layer)
  {
    layer->setName(name);
    layer->setFlags(static_cast<LayerFlags>(flags));
    layer->setId(id);
    layer->setUserData(userData);
  }

  return layer.release();
}

} // namespace doc
