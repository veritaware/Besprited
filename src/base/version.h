// Base Library
// Aseprite  | Copyright (C) 2001-2013, 2015 David Capello
// Besprited | Copyright (C) 2026            Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <string>
#include <vector>

namespace base
{

class Version
{
public:
  explicit Version(const std::string& from);

  bool operator<(const Version& other) const;

  [[nodiscard]] std::string str() const;

private:
  using Digits = std::vector<int>;
  Digits m_digits;
  std::string
      m_prerelease; // alpha, beta, dev, rc (empty if it's official release)
  int m_prereleaseDigit;
};

} // namespace base
