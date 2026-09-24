// Document Library
// Aseprite  | Copyright (C) 2001-2016 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include "doc/document_range.h"
#include "doc/frame.h"
#include "doc/layer_index.h"
#include <memory>

namespace doc
{

class Cel;
class Document;
class Image;
class Layer;
class Palette;
class Sprite;

// Specifies the current location in a context. E.g. the location in
// the current Editor (current document, sprite, layer, frame,
// etc.).
class Site
{
public:
  Site()
    : m_document(nullptr)
    , m_sprite(nullptr)
    , m_layer(nullptr)
    , m_frame(0)
  {
  }

  const Document* document() const { return m_document; }
  const Sprite* sprite() const { return m_sprite; }
  const Layer* layer() const { return m_layer; }
  frame_t frame() const { return m_frame; }
  // The range of selected cels/frames/layers (disabled if none).
  const DocumentRange& range() const { return m_range; }
  std::shared_ptr<const Cel> cel() const;

  Document* document() { return m_document; }
  Sprite* sprite() { return m_sprite; }
  Layer* layer() { return m_layer; }
  std::shared_ptr<Cel> cel();

  void document(Document* document) { m_document = document; }
  void sprite(Sprite* sprite) { m_sprite = sprite; }
  void layer(Layer* layer) { m_layer = layer; }
  void frame(frame_t frame) { m_frame = frame; }
  void range(const DocumentRange& range) { m_range = range; }

  LayerIndex layerIndex() const;
  void layerIndex(LayerIndex layerIndex);
  Palette* palette();
  Image* image(int* x = nullptr, int* y = nullptr,
               int* opacity = nullptr) const;
  Palette* palette() const;

private:
  Document* m_document;
  Sprite* m_sprite;
  Layer* m_layer;
  frame_t m_frame;
  DocumentRange m_range;
};

} // namespace doc
