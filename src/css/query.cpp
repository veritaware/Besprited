// CSS Library
// Aseprite  | Copyright (C) 2013 David Capello
// Besprited | Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "css/query.h"

namespace css
{

void Query::addFromStyle(const Style* style)
{
  for (const auto& entry : *style)
    addRuleValue(entry.first, style);
}

void Query::addRuleValue(const std::string& ruleName, const Style* style)
{
  if (!m_ruleValue.exists(ruleName))
    m_ruleValue.add(ruleName, style);
}

} // namespace css
