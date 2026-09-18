// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/cmd/copy_frame.h"

#include "app/cmd/add_frame.h"
#include "app/cmd/copy_cel.h"
#include "app/cmd/set_frame_duration.h"
#include "doc/sprite.h"
#include "doc/layer.h"

namespace app::cmd
{

using namespace doc;

CopyFrame::CopyFrame(const Sprite* sprite, const frame_t fromFrame,
                     const frame_t newFrame)
  : WithSprite(sprite)
  , m_fromFrame(fromFrame)
  , m_newFrame(newFrame)
{
}

void CopyFrame::onExecute()
{
  const Sprite* sprite = this->sprite();
  frame_t fromFrame = m_fromFrame;
  const int msecs = sprite->frameDuration(fromFrame);

  executeAndAdd(new AddFrame(sprite, m_newFrame));
  executeAndAdd(new SetFrameDuration(sprite, m_newFrame, msecs));

  if (fromFrame >= m_newFrame)
    ++fromFrame;

  for (int i = 0; i < sprite->countLayers(); ++i)
  {
    if (Layer* layer = sprite->layer(i); layer->isImage())
    {
      executeAndAdd(new CopyCel(dynamic_cast<LayerImage*>(layer), fromFrame,
                                dynamic_cast<LayerImage*>(layer), m_newFrame,
                                layer->isContinuous()));
    }
  }
}

} // namespace app::cmd
