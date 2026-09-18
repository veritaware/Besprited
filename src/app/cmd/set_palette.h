// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "app/cmd.h"
#include "app/cmd/with_sprite.h"
#include "doc/color.h"
#include "doc/frame.h"

#include <vector>

namespace doc
{
class Palette;
class Sprite;
}

namespace app::cmd
{
using namespace doc;

class SetPalette : public Cmd,
                   public WithSprite
{
public:
  SetPalette(const Sprite* sprite, frame_t frame, const Palette& newPalette);

protected:
  void onExecute() override;
  void onUndo() override;
  [[nodiscard]] size_t onMemSize() const override
  {
    return sizeof(*this) +
           sizeof(color_t) * (m_oldColors.size() + m_newColors.size());
  }

private:
  frame_t m_frame;
  int m_from, m_to;
  int m_oldNColors;
  int m_newNColors;
  std::vector<color_t> m_oldColors;
  std::vector<color_t> m_newColors;
};

} // namespace app::cmd
