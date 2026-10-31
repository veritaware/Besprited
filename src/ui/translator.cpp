// Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ui/translator.h"

namespace ui
{

static Translator g_translator;

void setTranslator(Translator translator)
{
  g_translator = std::move(translator);
}

std::string translate(std::string_view key, std::string_view fallback)
{
  if (g_translator)
    return g_translator(key, fallback);
  return std::string(fallback);
}

} // namespace ui
