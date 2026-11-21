// Numeric utils library
// Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <concepts>
#include <utility>

namespace base
{

template <typename T>
concept arithmetic = std::integral<T> or std::floating_point<T>;

enum class RangeBound
{
  Closed,
  Open,
  LeftOpen,
  RightOpen
};

template <typename T>
  requires arithmetic<T>
constexpr bool in_range(const T& value, const T& min, const T& max, RangeBound bound = RangeBound::Closed) noexcept
{
  switch (bound)
  {
  case RangeBound::Closed:
    return min <= value && max >= value;
  case RangeBound::Open:
    return min < value && max > value;
  case RangeBound::LeftOpen:
    return min < value && max >= value;
  case RangeBound::RightOpen:
    return min <= value && max > value;
  }
  std::unreachable();
}

} // namespace base
