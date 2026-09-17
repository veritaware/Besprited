// UI Library
// Aseprite  | Copyright (C) 2001-2013 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include "ui/message_type.h"

namespace ui
{

class RegisterMessage
{
public:
  RegisterMessage();
  operator MessageType() { return m_type; }

private:
  MessageType m_type;
};

} // namespace ui
