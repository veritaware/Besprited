// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.
#pragma once

#include "app/cmd_sequence.h"
#include "app/cmd/with_sprite.h"
#include "doc/frame.h"

namespace app::cmd
{
using namespace doc;

class CopyFrame : public CmdSequence,
                  public WithSprite
{
public:
  CopyFrame(const Sprite* sprite, frame_t fromFrame, frame_t newFrame);

protected:
  void onExecute() override;
  [[nodiscard]] size_t onMemSize() const override
  {
    return sizeof(*this) + CmdSequence::onMemSize() - sizeof(CmdSequence);
  }

private:
  frame_t m_fromFrame;
  frame_t m_newFrame;
};

} // namespace app::cmd
