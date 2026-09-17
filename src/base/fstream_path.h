// Base Library
// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.
#pragma once

#include "base/string.h"

#ifdef _WIN32
#ifdef __MINGW32__
#define FSTREAM_PATH(path) (std::string(path).c_str())
#else
#define FSTREAM_PATH(path) (base::from_utf8(path).c_str())
#endif
#else
#define FSTREAM_PATH(path) (std::string(path).c_str())
#endif
