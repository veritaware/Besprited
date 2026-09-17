// UI Library
// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include "gfx/color.h"
#include "ui/widget.h"

namespace ui
{

class Label : public Widget
{
public:
  Label(const std::string& text);

protected:
  void onSizeHint(SizeHintEvent& ev) override;
  void onPaint(PaintEvent& ev) override;
};

} // namespace ui
