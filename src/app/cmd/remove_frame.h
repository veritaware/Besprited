// Aseprite  | Copyright (C) 2001-2016 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "app/cmd.h"
#include "app/cmd/with_sprite.h"
#include "app/cmd_sequence.h"
#include "doc/frame.h"

namespace app::cmd
{
using namespace doc;

class RemoveFrame : public Cmd,
                    public WithSprite
{
public:
  RemoveFrame(const Sprite* sprite, frame_t frame);

protected:
  void onExecute() override;
  void onUndo() override;
  [[nodiscard]] size_t onMemSize() const override
  {
    return sizeof(*this) + m_seq.memSize();
  }

private:
  frame_t m_frame;
  int m_frameDuration;
  CmdSequence m_seq;
  bool m_firstTime;
};

} // namespace app::cmd
