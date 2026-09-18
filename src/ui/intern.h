// UI Library
// Aseprite  | Copyright (C) 2001-2013, 2015 David Capello
// Besprited | Copyright (C) 2026            Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include "gfx/color.h"
#include "ui/base.h"

namespace she
{
class Font;
}

namespace ui
{

class Graphics;
class Widget;
class Window;

// intern.cpp

namespace details
{

void initWidgets();
void exitWidgets();

void addWidget(Widget* widget);
void removeWidget(Widget* widget);

void resetFontAllWidgets();
void reinitThemeForAllWidgets();

} // namespace details

// theme.cpp

void drawTextBox(Graphics* g, Widget* textbox, int* w, int* h, gfx::Color bg,
                 gfx::Color fg);

} // namespace ui
