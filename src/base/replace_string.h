// Base Library
// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <string>

namespace base
{

void replace_string(std::string& subject, const std::string& replace_this,
                    const std::string& with_that);

}
