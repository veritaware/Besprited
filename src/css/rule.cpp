// CSS Library
// Aseprite  | Copyright (C) 2013 David Capello
// Besprited | Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "css/rule.h"

#include <utility>

namespace css
{

Rule::Rule(std::string name)
  : m_name(std::move(name))
{
}

} // namespace css
