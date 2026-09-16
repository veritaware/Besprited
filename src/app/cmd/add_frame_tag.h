// Aseprite  | Copyright (C) 2001-2016 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "app/cmd.h"
#include "app/cmd/with_frame_tag.h"
#include "app/cmd/with_sprite.h"

#include <sstream>

namespace app::cmd
{
using namespace doc;

class AddFrameTag : public Cmd,
                    public WithSprite,
                    public WithFrameTag
{
public:
  AddFrameTag(const Sprite* sprite, const FrameTag* tag);

protected:
  void onExecute() override;
  void onUndo() override;
  void onRedo() override;
  size_t onMemSize() const override { return sizeof(*this) + m_size; }

private:
  size_t m_size;
  std::stringstream m_stream;
};
} // namespace app::cmd
