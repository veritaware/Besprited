// Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <functional>
#include <string>
#include <string_view>

namespace ui
{

using Translator = std::function<std::string(std::string_view key, std::string_view fallback)>;

// Transitional hook: installed by the application so ui can translate
// loader-provided texts without depending on it.
void setTranslator(Translator translator);

// Returns the translation of key, or fallback if there is none.
std::string translate(std::string_view key, std::string_view fallback);

} // namespace ui
