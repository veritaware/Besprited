// UI Library
// Aseprite  | Copyright (C) 2001-2013, 2015 David Capello
// Besprited | Copyright (C) 2026            Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include "ui/event.h"

namespace ui
{

class Graphics;
class Widget;

class PaintEvent : public Event
{
public:
  PaintEvent(Widget* source, Graphics* graphics);
  ~PaintEvent() override;

  Graphics* graphics();

  bool isPainted() const;

private:
  Graphics* m_graphics;
  bool m_painted;
};

} // namespace ui
