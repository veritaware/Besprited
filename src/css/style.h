// CSS Library
// Aseprite  | Copyright (C) 2013 David Capello
// Besprited | Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <string>

#include "css/map.h"
#include "css/rule.h"
#include "css/value.h"

namespace css
{

class Style
{
public:
  using iterator = Values::iterator;
  using const_iterator = Values::const_iterator;

  Style() = default;
  Style(std::string name, const Style* base = nullptr);

  [[nodiscard]] const std::string& name() const { return m_name; }
  [[nodiscard]] const Style* base() const { return m_base; }

  const Value& operator[](const Rule& rule) const
  {
    return m_values[rule.name()];
  }

  Value& operator[](const Rule& rule) { return m_values[rule.name()]; }

  [[nodiscard]] iterator begin() { return m_values.begin(); }
  [[nodiscard]] iterator end() { return m_values.end(); }
  [[nodiscard]] const_iterator begin() const { return m_values.begin(); }
  [[nodiscard]] const_iterator end() const { return m_values.end(); }

private:
  std::string m_name;
  const Style* m_base;
  Values m_values;
};

using Styles = Map<const Style*>;

} // namespace css
