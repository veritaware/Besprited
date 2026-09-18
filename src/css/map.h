// CSS Library
// Aseprite  | Copyright (C) 2013 David Capello
// Besprited | Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <map>
#include <string>

namespace css
{

template <typename T> class Map
{
public:
  using map = std::map<std::string, T>;
  using iterator = typename map::iterator;
  using const_iterator = typename map::const_iterator;

  Map()
    : m_default()
  {
  }

  [[nodiscard]] iterator begin() { return m_map.begin(); }
  [[nodiscard]] iterator end() { return m_map.end(); }

  [[nodiscard]] const_iterator begin() const { return m_map.begin(); }
  [[nodiscard]] const_iterator end() const { return m_map.end(); }

  const T& operator[](const std::string& name) const
  {
    const auto it = m_map.find(name);
    if (it != m_map.end())
      return it->second;
    else
      return m_default;
  }

  T& operator[](const std::string& name)
  {
    const auto it = m_map.find(name);
    if (it != m_map.end())
      return it->second;
    else
      // cppcheck-suppress internalAstError ; cppcheck's parser misreads this
      // chained assign-through-operator[] in template code; it's valid C++.
      return m_map[name] = T();
  }

  void add(const std::string& name, T value) { m_map[name] = value; }

  [[nodiscard]] bool exists(const std::string& name) const
  {
    return (m_map.find(name) != m_map.end());
  }

private:
  map m_map;
  T m_default;
};

} // namespace css
