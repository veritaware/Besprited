// Document Library
// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

namespace doc
{

enum class ObjectType
{
  Unknown,
  Image,
  Palette,
  RgbMap,
  Path,
  Mask,
  Cel,
  CelData,
  LayerImage,
  LayerFolder,
  Sprite,
  Document,
  FrameTag,
};

} // namespace doc
