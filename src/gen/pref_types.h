// Code Generator
// Aseprite  | Copyright (C) 2014 David Capello
// Besprited | Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <string>
#include "tinyxml2.h"

void gen_pref_header(tinyxml2::XMLDocument* doc, const std::string& inputFn);
void gen_pref_impl(tinyxml2::XMLDocument* doc, const std::string& inputFn);
