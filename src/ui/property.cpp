// UI Library
// Aseprite  | Copyright (C) 2001-2013 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "ui/property.h"

#include <utility>

namespace ui
{

Property::Property(std::string name)
  : m_name(std::move(name))
{
}

Property::~Property() = default;

std::string Property::getName() const
{
  return m_name;
}

} // namespace ui
