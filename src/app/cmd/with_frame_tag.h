// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "doc/object_id.h"

namespace doc
{
class FrameTag;
}

namespace app::cmd
{
using namespace doc;

class WithFrameTag
{
public:
  explicit WithFrameTag(const FrameTag* tag);
  [[nodiscard]] FrameTag* frameTag() const;

private:
  ObjectId m_frameTagId;
};

} // namespace app::cmd
