// UI Library
// Aseprite  | Copyright (C) 2001-2014 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include "ui/widget.h"

namespace ui
{

class Panel : public Widget
{
public:
  Panel();

  void showChild(Widget* widget);

protected:
  void onResize(ResizeEvent& ev) override;
  void onSizeHint(SizeHintEvent& ev) override;
};

} // namespace ui
