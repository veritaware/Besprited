// Base Library
// Aseprite  | Copyright (C) 2015-2016 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include "base/ints.h"

namespace base
{

using pid = uint32_t;

pid get_current_process_id();

bool is_process_running(pid pid);

} // namespace base
