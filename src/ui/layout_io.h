// UI Library
// Aseprite  | Copyright (C) 2001-2013 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <string>

namespace ui
{

class Widget;

class LayoutIO
{
public:
  virtual ~LayoutIO() = default;
  virtual std::string loadLayout(Widget* widget) = 0;
  virtual void saveLayout(Widget* widget, const std::string& str) = 0;
};

} // namespace ui
