// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "app/commands/command.h"

namespace doc
{
class Palette;
}

namespace app
{

class SetPaletteCommand : public Command
{
public:
  enum class Target
  {
    Document,
    App
  };

  SetPaletteCommand();

  // With resetTransparentIndex the palette gets a transparent first entry
  // (see Palette::ensureTransparentFirst) and an indexed sprite's
  // transparent index becomes 0. Applies to the next execution only.
  void setPalette(const doc::Palette* palette, bool resetTransparentIndex = false)
  {
    m_palette = palette;
    m_resetTransparentIndex = resetTransparentIndex;
  }

protected:
  void onExecute(Context* context) override;

private:
  const doc::Palette* m_palette;
  bool m_resetTransparentIndex = false;
};

} // namespace app
