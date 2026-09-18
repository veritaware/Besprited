// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/cmd/with_frame_tag.h"

#include "doc/frame_tag.h"

namespace app::cmd
{

using namespace doc;

WithFrameTag::WithFrameTag(const FrameTag* tag)
  : m_frameTagId(tag->id())
{
}

FrameTag* WithFrameTag::frameTag() const
{
  return get<FrameTag>(m_frameTagId);
}

} // namespace app::cmd
