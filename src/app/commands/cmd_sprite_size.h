// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "app/commands/command.h"
#include "doc/algorithm/resize_image.h"

#include <string>

namespace ui
{
class CheckBox;
class Entry;
}

namespace app
{

class SpriteSizeCommand : public Command
{
public:
  SpriteSizeCommand();

  void setScale(double x, double y)
  {
    m_scaleX = x;
    m_scaleY = y;
  }

protected:
  void onLoadParams(const Params& params) override;
  bool onEnabled(Context* context) override;
  void onExecute(Context* context) override;

private:
  bool m_useUI;
  int m_width;
  int m_height;
  double m_scaleX;
  double m_scaleY;
  doc::algorithm::ResizeMethod m_resizeMethod;
};

} // namespace app
