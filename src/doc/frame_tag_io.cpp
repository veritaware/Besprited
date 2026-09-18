// Document Library
// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "doc/frame_tag_io.h"

#include "base/serialization.h"
#include "doc/frame_tag.h"
#include "doc/string_io.h"

#include <iostream>
#include <memory>

namespace doc
{

using namespace base::serialization;
using namespace base::serialization::little_endian;

void write_frame_tag(std::ostream& os, const FrameTag* tag)
{
  const std::string name = tag->name();

  write32(os, tag->id());
  write32(os, tag->fromFrame());
  write32(os, tag->toFrame());
  write32(os, tag->color());
  write8(os, static_cast<int>(tag->aniDir()));
  write_string(os, tag->name());
}

FrameTag* read_frame_tag(std::istream& is, bool setId)
{
  const ObjectId id = read32(is);
  const frame_t from = read32(is);
  const frame_t to = read32(is);
  const color_t color = read32(is);
  auto aniDir = static_cast<AniDir>(read8(is));
  const std::string name = read_string(is);

  std::unique_ptr<FrameTag> tag = std::make_unique<FrameTag>(from, to);
  tag->setColor(color);
  tag->setAniDir(aniDir);
  tag->setName(name);
  if (setId)
    tag->setId(id);
  return tag.release();
}

} // namespace doc
