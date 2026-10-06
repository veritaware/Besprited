// Config Library
// Besprited | Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#ifdef __EMSCRIPTEN__

#include <string>

// Browser-side (IndexedDB-backed) settings storage, implemented in JS.

// Loads the stored settings into memory; false if storage isn't ready yet.
bool cfginit();

std::string cfg_wasm_load(const std::string& filename);
void cfg_wasm_save(const std::string& filename, const std::string& data);

#endif
