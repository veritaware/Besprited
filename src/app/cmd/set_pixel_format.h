// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "app/cmd_sequence.h"
#include "app/cmd/with_sprite.h"
#include "doc/pixel_format.h"
#include "doc/dithering_method.h"

namespace doc
{
class Sprite;
}

namespace app::cmd
{
using namespace doc;

class SetPixelFormat : public Cmd,
                       public WithSprite
{
public:
  SetPixelFormat(Sprite* sprite, PixelFormat newFormat,
                 DitheringMethod dithering);

protected:
  void onExecute() override;
  void onUndo() override;
  void onRedo() override;
  [[nodiscard]] size_t onMemSize() const override
  {
    return sizeof(*this) + m_seq.memSize();
  }

private:
  void setFormat(PixelFormat format);

  PixelFormat m_oldFormat;
  PixelFormat m_newFormat;
  DitheringMethod m_dithering;
  CmdSequence m_seq;
};

} // namespace app::cmd
