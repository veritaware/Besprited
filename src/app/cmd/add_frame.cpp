// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/cmd/add_frame.h"

#include <memory>

#include "app/cmd/add_cel.h"
#include "app/document.h"
#include "doc/cel.h"
#include "doc/document_event.h"
#include "doc/layer.h"
#include "doc/primitives.h"
#include "doc/sprite.h"

namespace app::cmd
{

using namespace doc;

AddFrame::AddFrame(const Sprite* sprite, const frame_t frame)
  : WithSprite(sprite)
  , m_newFrame(frame)
  , m_addCel(nullptr)
{
}

void AddFrame::onExecute()
{
  Sprite* sprite = this->sprite();
  auto* doc = dynamic_cast<Document*>(sprite->document());

  sprite->addFrame(m_newFrame);
  sprite->incrementVersion();

  if (m_addCel)
  {
    m_addCel->redo();
  }
  else
  {
    if (LayerImage* bg_layer = sprite->backgroundLayer())
    {
      const ImageRef bg_image(Image::create(sprite->pixelFormat(),
                                            sprite->width(), sprite->height()));
      clear_image(bg_image.get(), doc->bgColor(bg_layer));
      const auto cel = std::make_shared<Cel>(m_newFrame, bg_image);
      m_addCel = std::make_unique<AddCel>(bg_layer, cel);
      m_addCel->execute(context());
    }
  }

  // Notify observers about the new frame.
  DocumentEvent ev(doc);
  ev.sprite(sprite);
  ev.frame(m_newFrame);
  doc->notifyObservers<DocumentEvent&>(&DocumentObserver::onAddFrame, ev);
}

void AddFrame::onUndo()
{
  Sprite* sprite = this->sprite();
  auto* doc = dynamic_cast<Document*>(sprite->document());

  if (m_addCel)
    m_addCel->undo();

  sprite->removeFrame(m_newFrame);
  sprite->incrementVersion();

  // Notify observers about the new frame.
  DocumentEvent ev(doc);
  ev.sprite(sprite);
  ev.frame(m_newFrame);
  doc->notifyObservers<DocumentEvent&>(&DocumentObserver::onRemoveFrame, ev);
}

} // namespace app::cmd
