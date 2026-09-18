// Base Library
// Aseprite  | Copyright (C) 2016 David Capello
// Besprited | Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <string>

namespace base
{

using dll = void*;
using dll_proc = void*;

dll load_dll(const std::string& filename);
void unload_dll(dll lib);
dll_proc get_dll_proc_base(dll lib, const char* procName);

template <typename T> inline T get_dll_proc(dll lib, const char* procName)
{
  return reinterpret_cast<T>(get_dll_proc_base(lib, procName));
}

} // namespace base
