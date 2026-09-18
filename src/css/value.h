// CSS Library
// Aseprite  | Copyright (C) 2013 David Capello
// Besprited | Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include "css/map.h"

#include <cstdint>
#include <string>

namespace css
{

class Value
{
public:
  enum Type : std::uint8_t
  {
    None,
    Number,
    String
  };

  Value();
  explicit Value(double value, std::string unit = "");
  explicit Value(std::string value);

  [[nodiscard]] Type type() const { return m_type; }

  [[nodiscard]] double number() const;
  [[nodiscard]] std::string string() const;
  [[nodiscard]] std::string unit() const;

  void setNumber(double value);
  void setString(std::string value);
  void setUnit(const std::string& unit = "");

  bool operator==(const Value& other) const;
  bool operator!=(const Value& other) const { return !operator==(other); }

private:
  Type m_type;
  double m_number;
  std::string m_string;
};

using Values = Map<Value>;

} // namespace css
