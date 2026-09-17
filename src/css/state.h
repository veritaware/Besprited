// CSS Library
// Aseprite  | Copyright (C) 2013 David Capello
// Besprited | Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <string>
#include <utility>
#include <vector>

namespace css
{

class State
{
public:
  State() = default;
  State(std::string name)
    : m_name(std::move(name))
  {
  }

  [[nodiscard]] const std::string& name() const { return m_name; }

private:
  std::string m_name;
};

class States
{
public:
  using List = std::vector<const State*>;
  using iterator = List::iterator;
  using const_iterator = List::const_iterator;
  using reverse_iterator = List::reverse_iterator;
  using const_reverse_iterator = List::const_reverse_iterator;

  States() = default;
  States(const State& state) { operator+=(state); }

  [[nodiscard]] iterator begin() { return m_list.begin(); }
  [[nodiscard]] iterator end() { return m_list.end(); }
  [[nodiscard]] const_iterator begin() const { return m_list.begin(); }
  [[nodiscard]] const_iterator end() const { return m_list.end(); }
  [[nodiscard]] reverse_iterator rbegin() { return m_list.rbegin(); }
  [[nodiscard]] reverse_iterator rend() { return m_list.rend(); }
  [[nodiscard]] const_reverse_iterator rbegin() const
  {
    return m_list.rbegin();
  }
  [[nodiscard]] const_reverse_iterator rend() const { return m_list.rend(); }

  States& operator+=(const State& other)
  {
    m_list.push_back(&other);
    return *this;
  }

  States& operator+=(const States& others)
  {
    for (const State* state : others)
      operator+=(*state);
    return *this;
  }

  bool operator<(const States& other) const { return m_list < other.m_list; }

private:
  List m_list;
};

inline States operator+(const State& a, const State& b)
{
  States states;
  states += a;
  states += b;
  return states;
}

} // namespace css
