// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "app/cmd.h"
#include "app/cmd/with_sprite.h"
#include "doc/remap.h"

namespace app::cmd
{
using namespace doc;

class RemapColors : public Cmd,
                    public WithSprite
{
public:
  RemapColors(const Sprite* sprite, Remap remap);

protected:
  void onExecute() override;
  void onUndo() override;
  [[nodiscard]] size_t onMemSize() const override
  {
    return sizeof(*this) + m_remap.getMemSize();
  }

private:
  static void incrementVersions(const Sprite* spr);

  Remap m_remap;
};

} // namespace app::cmd
