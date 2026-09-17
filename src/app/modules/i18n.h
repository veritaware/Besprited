// LibreSprite | Copyright (C) 2024 LibreSprite contributors
// Besprited   | Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <string>

namespace app
{
void setLanguage(const std::string& language);
const std::string& getLanguage();
std::string i18n(const std::string& key, const std::string& src);
inline std::string i18n(const std::string& key)
{
  return i18n(key, key);
}
} // namespace app
