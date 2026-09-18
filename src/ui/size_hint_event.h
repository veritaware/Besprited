// UI Library
// Aseprite  | Copyright (C) 2001-2013, 2015 David Capello
// Besprited | Copyright (C) 2026            Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include "gfx/size.h"
#include "ui/event.h"

namespace ui
{

class Widget;

class SizeHintEvent : public Event
{
public:
  SizeHintEvent(Widget* source, const gfx::Size& fitIn);
  ~SizeHintEvent() override;

  gfx::Size fitInSize() const;
  int fitInWidth() const;
  int fitInHeight() const;

  gfx::Size sizeHint() const;
  void setSizeHint(const gfx::Size& sz);
  void setSizeHint(int w, int h);

private:
  gfx::Size m_fitIn;
  gfx::Size m_sizeHint;
};

} // namespace ui
