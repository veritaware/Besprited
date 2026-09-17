// UI Library
// Aseprite  | Copyright (C) 2001-2013, 2015 David Capello
// Besprited | Copyright (C) 2026            Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include "base/disable_copying.h"
#include "base/shared_ptr.h"

#include <string>

namespace ui
{

class Property
{
public:
  Property(std::string name);
  virtual ~Property();

  std::string getName() const;

private:
  std::string m_name;

  DISABLE_COPYING(Property);
};

using PropertyPtr = base::SharedPtr<Property>;

} // namespace ui
