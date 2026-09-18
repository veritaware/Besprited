// LibreSprite | Copyright (C) 2024 LibreSprite contributors
// Besprited   | Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once
#include <iterator>

namespace base
{
template <typename iterator> class range
{
  std::pair<iterator, iterator> pair;

public:
  using iterator_category = typename iterator::iterator_category;
  using value_type = typename iterator::value_type;
  using difference_type = typename iterator::difference_type;
  using pointer = typename iterator::pointer;
  using reference = typename iterator::reference;

  range(const std::pair<iterator, iterator>& pair)
    : pair{pair}
  {
  }

  iterator begin() { return pair.first; }

  iterator end() { return pair.second; }

  bool empty() { return begin() == end(); }
};
} // namespace base
