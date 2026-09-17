// UI Library
// Aseprite  | Copyright (C) 2001-2013 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ui/custom_label.h"

namespace ui
{

CustomLabel::CustomLabel(const std::string& text)
  : Label(text)
{
}

bool CustomLabel::onProcessMessage(Message* msg)
{
  return Label::onProcessMessage(msg);
}

} // namespace ui
