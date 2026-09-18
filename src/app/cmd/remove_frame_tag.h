// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "app/cmd/add_frame_tag.h"

namespace app::cmd
{
using namespace doc;

class RemoveFrameTag : public AddFrameTag
{
public:
  RemoveFrameTag(const Sprite* sprite, const FrameTag* tag);

protected:
  void onExecute() override;
  void onUndo() override;
  void onRedo() override;
};

} // namespace app::cmd
