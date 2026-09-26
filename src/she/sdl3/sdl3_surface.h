// SHE library
// Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

// SDL3 counterpart of she/sdl2/sdl2_surface.h - see #73.

#include "she/surface.h"
#include "she/common/generic_surface.h"
#include "she/sdl3/sdl3_display.h"

#include <cstdint>

struct SDL_Surface;
struct SDL_PixelFormatDetails;

namespace she
{

class SDL3Surface
  : public GenericDrawTextSurface<GenericDrawColoredRgbaSurface<Surface>>
{
public:
  enum [[clang::flag_enum]] DestroyFlag : std::uint8_t
  {
    None = 0,
    DeleteThis = 1,
    DestroyHandle = 2,
    DeleteAndDestroy = DeleteThis | DestroyHandle,
  };

  SDL3Surface(SDL_Surface* bmp, DestroyFlag destroy);
  SDL3Surface(int width, int height, DestroyFlag destroy);
  SDL3Surface(int width, int height, int bpp, DestroyFlag destroy);
  ~SDL3Surface() override;

  // Surface implementation
  void dispose() override;
  [[nodiscard]] int width() const override;
  [[nodiscard]] int height() const override;
  [[nodiscard]] bool isDirectToScreen() const override;
  gfx::Rect getClipBounds() override;
  void setClipBounds(const gfx::Rect& rc) override;
  bool intersectClipRect(const gfx::Rect& rc) override;
  void lock() override;
  void unlock() override;
  void setDrawMode(DrawMode mode, int param) override;
  void applyScale(int scale) override;
  void* nativeHandle() override;
  void clear() override;
  [[nodiscard]] uint8_t* getData(int x, int y) const override;
  void getFormat(SurfaceFormatData* formatData) const override;
  [[nodiscard]] gfx::Color getPixel(int x, int y) const override;
  void putPixel(gfx::Color color, int x, int y) override;
  void drawHLine(gfx::Color color, int x, int y, int w) override;
  void drawVLine(gfx::Color color, int x, int y, int h) override;
  void drawLine(gfx::Color color, const gfx::Point& a,
                const gfx::Point& b) override;
  void drawRect(gfx::Color color, const gfx::Rect& rc) override;
  void fillRect(gfx::Color color, const gfx::Rect& rc) override;
  void blitTo(Surface* dest, int srcx, int srcy, int dstx, int dsty, int width,
              int height) const override;
  void scrollTo(const gfx::Rect& rc, int dx, int dy) override;
  void drawSurface(const Surface* src, int dstx, int dsty) override;
  void drawRgbaSurface(const Surface* src, int dstx, int dsty) override;

  SDL_Texture* getTexture(const SDL_Rect* rect = nullptr);

  static inline unsigned int textureGen;

private:
  // SDL3 surfaces carry a pixel-format enum value (not a struct pointer with
  // Rloss/Gloss/Bloss/Aloss like SDL2), so masks/shifts/bit-depths are looked
  // up on demand through this accessor instead of a cached struct pointer.
  [[nodiscard]] const SDL_PixelFormatDetails* fmt() const;

  unsigned int m_textureGen{};
  SDL_Texture* m_texture{};
  SDL_Surface* m_bmp{};
  DestroyFlag m_destroy;
  int m_lock;
};

} // namespace she
