// UI Library
// Aseprite  | Copyright (C) 2001-2013 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include "ui/widget.h"

namespace ui
{

class Viewport : public Widget
{
public:
  Viewport();

  gfx::Size calculateNeededSize();

protected:
  // Events
  void onResize(ResizeEvent& ev) override;
  void onSizeHint(SizeHintEvent& ev) override;
  void onPaint(PaintEvent& ev) override;
};

} // namespace ui
