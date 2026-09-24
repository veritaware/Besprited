// SHE library
// Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

// Phase 0 scaffolding of the SDL3 backend (see #73, #260). It only makes the
// backend configurable, compilable and linkable; the she::System,
// she::EventQueue and friends are ported in the following phases.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "she/she.h"

#include "she/keys.h"
#include "she/sdl3/sdl3_display.h"
#include "she/sdl3/sdl3_surface.h"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <iostream>
#include <string>

namespace ui {

  float get_pen_pressure() {
    return 0.0f; // TODO(#73)
  }

} // namespace ui

namespace she {

  void log(const std::string&) {
    // TODO(#73)
  }

  System* create_system() {
    return nullptr; // TODO(#73): SDL3System
  }

  System* instance() {
    return nullptr; // TODO(#73): SDL3System
  }

  void error_message(const char* msg) {
    std::cerr << msg << "\n";
  }

  int scancode_to_ascii(KeyScancode) {
    return 0; // TODO(#73)
  }

  bool is_key_pressed(KeyScancode) {
    return false; // TODO(#73)
  }

  void clear_keyboard_buffer() {
    // TODO(#73)
  }

  void set_input_rect(const gfx::Rect&) {
    // TODO(#73)
  }

} // namespace she

// It must be defined by the user program code.
extern int app_main(int argc, char* argv[]);

int main(const int, char*[]) {
  std::cerr << "The SDL3 backend is not functional yet (SDL3 migration, #73).\n"
            << "Build with -DUSE_SDL2_BACKEND=on for a working editor.\n";

  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
    std::cerr << "Critical: Could not initialize SDL3 (" << SDL_GetError() << "). Aborting.\n";
    return -1;
  }
  SDL_Quit();
  return 1;
}
