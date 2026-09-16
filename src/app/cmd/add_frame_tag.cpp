// Aseprite  | Copyright (C) 2001-2016 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/cmd/add_frame_tag.h"

#include "doc/frame_tag.h"
#include "doc/frame_tag_io.h"
#include "doc/sprite.h"

namespace app::cmd
{

using namespace doc;

AddFrameTag::AddFrameTag(const Sprite* sprite, const FrameTag* tag)
  : WithSprite(sprite)
  , WithFrameTag(tag)
  , m_size(0)
{
}

void AddFrameTag::onExecute()
{
  Sprite* sprite = this->sprite();
  FrameTag* frameTag = this->frameTag();

  sprite->frameTags().add(frameTag);
  sprite->incrementVersion();
}

void AddFrameTag::onUndo()
{
  Sprite* sprite = this->sprite();
  FrameTag* frameTag = this->frameTag();
  write_frame_tag(m_stream, frameTag);
  m_size = static_cast<size_t>(m_stream.tellp());

  sprite->frameTags().remove(frameTag);
  sprite->incrementVersion();
  delete frameTag;
}

void AddFrameTag::onRedo()
{
  Sprite* sprite = this->sprite();
  FrameTag* frameTag = read_frame_tag(m_stream);

  sprite->frameTags().add(frameTag);
  sprite->incrementVersion();

  m_stream.str(std::string());
  m_stream.clear();
  m_size = 0;
}

} // namespace app::cmd
