// Base Library
// Aseprite  | Copyright (C) 2001-2013, 2015 David Capello
// Besprited | Copyright (C) 2026            Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <cstddef>

void* base_malloc(std::size_t bytes);
void* base_malloc0(std::size_t bytes);
void* base_realloc(void* mem, std::size_t bytes);
void base_free(void* mem);
char* base_strdup(const char* string);

#ifdef MEMLEAK
void base_memleak_init();
void base_memleak_exit();
#endif
