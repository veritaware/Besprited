// SHE library
// Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

// SDL3 counterpart of she/sdl2/sdl2_display.h. Tablet (EasyTab/WM-info) and
// event-loop pieces are ported in later SDL3 migration phases - see #73.

#include "she/display.h"

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Surface;
struct SDL_Texture;
struct SDL_Cursor;
struct SDL_Rect;

namespace she
{

class SDL3Display final : public Display
{
  friend class SDL3System;

public:
  SDL3Display(int width, int height, int scale);
  ~SDL3Display();

  void dispose() override;
  void toggleFullscreen() override;
  bool setIcon(Surface*) override;
  [[nodiscard]] int width() const override;
  [[nodiscard]] int height() const override;
  void setWidth(int);
  void setHeight(int);
  [[nodiscard]] int originalWidth() const override;
  [[nodiscard]] int originalHeight() const override;
  void setOriginalWidth(int width);
  void setOriginalHeight(int height);
  [[nodiscard]] int scale() const override;
  void setScale(int scale) override;
  void recreateSurface();
  Surface* getSurface() override;
  void flip(const gfx::Rect& bounds) override;
  void maximize() override;
  [[nodiscard]] bool isMaximized() const override;
  [[nodiscard]] bool isMinimized() const override;
  void setTitleBar(const std::string& title) override;
  NativeCursor nativeMouseCursor() override;
  bool setNativeMouseCursor(NativeCursor cursor) override;
  void setMousePosition(const gfx::Point& position) override;
  void captureMouse() override;
  void releaseMouse() override;
  std::string getLayout() override;
  void setLayout(const std::string& layout) override;
  void* nativeHandle() override;

  void present() override;
  SDL_Renderer* renderer() { return m_renderer; }

  static inline bool gpu{};

private:
  SDL_Window* m_window;
  SDL_Renderer* m_renderer;
  Surface* m_surface{};
  Surface* m_doublebuffer{};
  int m_scale;
  int m_width;
  int m_height;
  NativeCursor m_nativeCursor;
  int m_restoredWidth;
  int m_restoredHeight;
  bool m_isFullscreen = false;
  bool m_dirty = true;
};

extern SDL3Display* unique_display;

} // namespace she
