// Base Library
// Aseprite  | Copyright (C) 2015 David Capello
// Besprited | Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include "base/buffer.h"

#include <string>

namespace base
{

void encode_base64(const buffer& input, std::string& output);
void decode_base64(const std::string& input, buffer& output);

} // namespace base
