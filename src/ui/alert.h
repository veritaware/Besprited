// UI Library
// Aseprite  | Copyright (C) 2001-2013, 2015 David Capello
// Besprited | Copyright (C) 2026            Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include "base/shared_ptr.h"
#include "ui/window.h"

namespace ui
{

class Box;
class Slider;

class Alert;
using AlertPtr = base::SharedPtr<Alert>;

class Alert : public Window
{
public:
  Alert();

  void addProgress();
  void setProgress(double progress);

  static AlertPtr create(const char* format, ...);
  static int show(const char* format, ...);

private:
  void processString(char* buf, std::vector<Widget*>& labels,
                     std::vector<Widget*>& buttons);

  Slider* m_progress;
  Box* m_progressPlaceholder;
};

} // namespace ui
