// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include <map>
#include <string>
#include <sstream>
#include <type_traits>

namespace app
{

class Params
{
public:
  using Map = std::map<std::string, std::string>;
  using iterator = Map::iterator;
  using const_iterator = Map::const_iterator;

  iterator begin() { return m_params.begin(); }
  iterator end() { return m_params.end(); }
  const_iterator begin() const { return m_params.begin(); }
  const_iterator end() const { return m_params.end(); }

  bool empty() const { return m_params.empty(); }

  void clear() { return m_params.clear(); }

  bool has_param(const char* name) const
  {
    return m_params.find(name) != m_params.end();
  }

  bool operator==(const Params& params) const
  {
    return m_params == params.m_params;
  }

  bool operator!=(const Params& params) const
  {
    return m_params != params.m_params;
  }

  std::string& set(const char* name, const char* value)
  {
    return m_params[name] = value;
  }

  // A failed lookup must not insert anything - callers hold onto Params
  // instances as persistent state (e.g. Key::params()) and compare them
  // for equality, so every const get() used to silently grow the map via
  // operator[] (which required `m_params` to be `mutable`), corrupting
  // that state (see issue #219, Phase 5).
  const std::string& get(const char* name) const
  {
    auto it = m_params.find(name);
    static const std::string empty;
    return it != m_params.end() ? it->second : empty;
  }

  void operator|=(const Params& params)
  {
    for (const auto& p : params)
      m_params[p.first] = p.second;
  }

  template <typename T> const T get_as(const char* name) const
  {
    auto it = m_params.find(name);
    if (it == m_params.end())
      return T();

    if constexpr (std::is_same_v<T, bool>)
    {
      // Plain `stream >> value` only recognizes "0"/"1" for bool (no
      // boolalpha), silently parsing "true"/"false" as false - several
      // callers worked around this by comparing the raw string instead of
      // using get_as<bool> at all.
      return it->second == "true" || it->second == "1";
    }
    else
    {
      std::istringstream stream(it->second);
      T value = T();
      stream >> value;
      return value;
    }
  }

private:
  Map m_params;
};

} // namespace app
