// Base Library
// Aseprite  | Copyright (C) 2001-2013, 2015 David Capello
// Besprited | Copyright (C) 2026            Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <string>
#include <vector>
#include <iterator>

namespace base
{

std::vector<std::string> split(const std::string& original, char delimiter);

std::string string_to_lower(const std::string& original);
std::string string_to_upper(const std::string& original);

std::string to_utf8(const std::wstring& widestring);
std::wstring from_utf8(const std::string& utf8string);

int utf8_length(const std::string& utf8string);
int utf8_icmp(const std::string& a, const std::string& b, int n = 0);

template <typename SubIterator> class utf8_iteratorT
{
public:
  // std::iterator<std::forward_iterator_tag, std::string::value_type,
  // std::string::difference_type, typename SubIterator::pointer, typename
  // SubIterator::reference>
  using iterator_category = std::forward_iterator_tag;
  using value_type = std::string::value_type;
  using difference_type = std::string::difference_type;
  using pointer = typename SubIterator::pointer;
  using reference = typename SubIterator::reference;

  explicit utf8_iteratorT(const SubIterator& it)
    : m_internal(it)
  {
  }

  // Based on Allegro Unicode code (allegro/src/unicode.c)
  utf8_iteratorT& operator++()
  {
    int c = static_cast<unsigned char>(*m_internal);
    ++m_internal;

    if (c & 0x80)
    {
      int n = 1;
      while (c & (0x80 >> n))
        n++;

      c &= (1 << (8 - n)) - 1;

      while (--n > 0)
      {
        const int t = static_cast<unsigned char>(*m_internal);
        ++m_internal;

        if ((!(t & 0x80)) || (t & 0x40))
        {
          --m_internal;
          return *this;
        }

        c = (c << 6) | (t & 0x3F);
      }
    }

    return *this;
  }

  utf8_iteratorT& operator+=(int i)
  {
    while (i--)
      operator++();
    return *this;
  }

  utf8_iteratorT operator+(int i)
  {
    utf8_iteratorT it(*this);
    it += i;
    return it;
  }

  const int operator*() const
  {
    SubIterator it = m_internal;
    int c = static_cast<unsigned char>(*it);
    ++it;

    if (c & 0x80)
    {
      int n = 1;
      while (c & (0x80 >> n))
        n++;

      c &= (1 << (8 - n)) - 1;

      while (--n > 0)
      {
        const int t = static_cast<unsigned char>(*it);
        ++it;

        if ((!(t & 0x80)) || (t & 0x40))
          return '^';

        c = (c << 6) | (t & 0x3F);
      }
    }

    return c;
  }

  bool operator==(const utf8_iteratorT& it) const
  {
    return m_internal == it.m_internal;
  }

  bool operator!=(const utf8_iteratorT& it) const
  {
    return m_internal != it.m_internal;
  }

  pointer operator->() { return m_internal.operator->(); }

  std::string::difference_type operator-(const utf8_iteratorT& it)
  {
    return m_internal - it.m_internal;
  }

private:
  SubIterator m_internal;
};

class utf8_iterator : public utf8_iteratorT<std::string::iterator>
{
public:
  utf8_iterator(const utf8_iteratorT<std::string::iterator>& it)
    : utf8_iteratorT<std::string::iterator>(it)
  {
  }
  explicit utf8_iterator(const std::string::iterator& it)
    : utf8_iteratorT<std::string::iterator>(it)
  {
  }
};

class utf8_const_iterator : public utf8_iteratorT<std::string::const_iterator>
{
public:
  utf8_const_iterator(const utf8_iteratorT<std::string::const_iterator>& it)
    : utf8_iteratorT<std::string::const_iterator>(it)
  {
  }
  explicit utf8_const_iterator(const std::string::const_iterator& it)
    : utf8_iteratorT<std::string::const_iterator>(it)
  {
  }
};

} // namespace base
