// CSS Library
// Aseprite  | Copyright (C) 2013 David Capello
// Besprited | Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "css/style.h"

#include <utility>

namespace css
{

Style::Style(std::string name, const Style* base)
  : m_name(std::move(name))
  , m_base(base)
{
}

} // namespace css
