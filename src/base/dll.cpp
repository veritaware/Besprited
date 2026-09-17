// Base Library
// Aseprite  | Copyright (C) 2016 David Capello
// Besprited | Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "base/dll.h"

#ifdef _WIN32
#include "base/dll_win32.h"
#else
#include "base/dll_unix.h"
#endif
