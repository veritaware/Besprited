// CSS Library
// Aseprite  | Copyright (C) 2013 David Capello
// Besprited | Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include "css/map.h"

#include <string>

namespace css
{

class Rule
{
public:
  Rule() = default;
  Rule(std::string name);

  [[nodiscard]] const std::string& name() const { return m_name; }

private:
  std::string m_name;
};

using Rules = Map<Rule*>;

} // namespace css
