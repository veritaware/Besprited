// Base Library
// Aseprite  | Copyright (C) 2001-2013, 2015 David Capello
// Besprited | Copyright (C) 2026            Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <list>
#include <algorithm>

namespace base
{

template <typename T> class RecentItems
{
public:
  using Items = std::list<T>;
  using iterator = typename Items::iterator;
  using const_iterator = typename Items::const_iterator;

  RecentItems(std::size_t limit)
    : m_limit(limit)
  {
  }

  const_iterator begin() { return m_items.begin(); }
  const_iterator end() { return m_items.end(); }

  bool empty() const { return m_items.empty(); }
  std::size_t size() const { return m_items.size(); }
  std::size_t limit() const { return m_limit; }

  template <typename T2, typename Predicate>
  void addItem(const T2& item, Predicate p)
  {
    iterator it = std::find_if(m_items.begin(), m_items.end(), p);

    // If the item already exist in the list...
    if (it != m_items.end())
    {
      // Move it to the first position
      m_items.erase(it);
      m_items.insert(m_items.begin(), item);
      return;
    }

    // Does the list is full?
    if (m_items.size() == m_limit)
    {
      // Remove the last entry
      m_items.erase(--m_items.end());
    }

    m_items.insert(m_items.begin(), item);
  }

  template <typename T2, typename Predicate>
  void removeItem(const T2& item, Predicate p)
  {
    iterator it = std::find_if(m_items.begin(), m_items.end(), p);
    if (it != m_items.end())
      m_items.erase(it);
  }

private:
  Items m_items;
  std::size_t m_limit;
};

} // namespace base
