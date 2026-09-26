// SHE library
// Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "she/sdl3/sdl3_surface.h"

#include "base/string.h"
#include "gfx/point.h"
#include "gfx/rect.h"
#include <cstdlib>
#include <iostream>
#include <SDL3/SDL.h>

namespace
{

she::DrawMode drawMode = she::DrawMode::Solid;
int checkedModeOffset = 0;

}

namespace she
{

namespace sdl
{
extern SDL3Surface* screen;
extern SDL3Surface* tempSurface;
}

inline gfx::Color from_sdl(const SDL_PixelFormatDetails* format, unsigned int color)
{
  return gfx::rgba((color & format->Rmask) >> format->Rshift << (8 - format->Rbits),
                   (color & format->Gmask) >> format->Gshift << (8 - format->Gbits),
                   (color & format->Bmask) >> format->Bshift << (8 - format->Bbits),
                   (color & format->Amask) >> format->Ashift << (8 - format->Abits));
}

inline unsigned int to_sdl(const SDL_PixelFormatDetails* format, gfx::Color color)
{
  return SDL_MapRGBA(format, nullptr, gfx::getr(color),
                     gfx::getg(color), gfx::getb(color), gfx::geta(color));
}

const SDL_PixelFormatDetails* SDL3Surface::fmt() const
{
  return SDL_GetPixelFormatDetails(m_bmp->format);
}

SDL3Surface::SDL3Surface(SDL_Surface* bmp, DestroyFlag destroy)
  : m_bmp(bmp)
  , m_destroy(destroy)
  , m_lock(0)
{
}

SDL3Surface::SDL3Surface(int width, int height, DestroyFlag destroy)
  : m_bmp(SDL_CreateSurface(
        width, height,
        SDL_GetPixelFormatForMasks(32, 0xFF, 0xFF00, 0xFF0000, 0xFF000000)))
  , m_destroy(destroy)
  , m_lock(0)
{
  if (!m_bmp)
  {
    throw std::runtime_error("Failed to create surface");
  }
}

SDL3Surface::SDL3Surface(int width, int height, int bpp, DestroyFlag destroy)
  : m_bmp(SDL_CreateSurface(
        width, height,
        SDL_GetPixelFormatForMasks(bpp, 0xFF, 0xFF00, 0xFF0000,
                                   bpp == 32 ? 0xFF000000 : 0)))
  , m_destroy(destroy)
  , m_lock(0)
{
  if (!m_bmp)
  {
    throw std::runtime_error("Failed to create surface");
  }
}

SDL3Surface::~SDL3Surface()
{
  ASSERT(m_lock == 0);
  if (m_destroy & DestroyHandle)
  {
    if (m_bmp)
      SDL_DestroySurface(m_bmp);
  }
  if (m_texture)
    SDL_DestroyTexture(m_texture);
}

// Surface implementation

void SDL3Surface::dispose()
{
  if (m_destroy & DeleteThis)
    delete this;
}

int SDL3Surface::width() const
{
  return m_bmp->w;
}

int SDL3Surface::height() const
{
  return m_bmp->h;
}

bool SDL3Surface::isDirectToScreen() const
{
  return false;
}

gfx::Rect SDL3Surface::getClipBounds()
{
  SDL_Rect rect;
  SDL_GetSurfaceClipRect(m_bmp, &rect);
  return {rect.x, rect.y, rect.w, rect.h};
}

void SDL3Surface::setClipBounds(const gfx::Rect& rc)
{
  const SDL_Rect rect{.x = rc.x, .y = rc.y, .w = rc.w, .h = rc.h};
  SDL_SetSurfaceClipRect(m_bmp, &rect);
}

bool SDL3Surface::intersectClipRect(const gfx::Rect& rc)
{
  auto bounds = getClipBounds();
  bounds &= rc;
  setClipBounds(bounds);
  return !bounds.isEmpty();
}

void SDL3Surface::lock()
{
  ASSERT(m_lock >= 0);
  if (m_lock++ == 0)
  {
    if (SDL_MUSTLOCK(m_bmp))
    {
      SDL_LockSurface(m_bmp);
    }
  }
}

void SDL3Surface::unlock()
{
  ASSERT(m_lock > 0);
  if (--m_lock == 0)
  {
    if (SDL_MUSTLOCK(m_bmp))
    {
      SDL_UnlockSurface(m_bmp);
    }
  }
}

void SDL3Surface::setDrawMode(DrawMode mode, int param)
{
  drawMode = mode;
  if (mode == she::DrawMode::Checked)
    checkedModeOffset = param;
}

void SDL3Surface::applyScale(int scale)
{
  if (scale < 2)
    return;

  SDL_Surface* scaled =
      SDL_CreateSurface(m_bmp->w * scale, m_bmp->h * scale, m_bmp->format);

  SDL_Rect drect{.x = 0, .y = 0, .w = scaled->w, .h = scaled->h};
  SDL_BlitSurfaceScaled(m_bmp, nullptr, scaled, &drect, SDL_SCALEMODE_NEAREST);

  if (m_destroy & DestroyHandle)
    SDL_DestroySurface(m_bmp);

  m_bmp = scaled;
  m_destroy = DestroyHandle;
}

void* SDL3Surface::nativeHandle()
{
  return reinterpret_cast<void*>(m_bmp);
}

void SDL3Surface::clear()
{
  SDL_FillSurfaceRect(m_bmp, nullptr, 0);
}

uint8_t* SDL3Surface::getData(int x, int y) const
{
  return reinterpret_cast<uint8_t*>(m_bmp->pixels) + y * m_bmp->pitch +
         x * fmt()->bytes_per_pixel;
}

void SDL3Surface::getFormat(SurfaceFormatData* formatData) const
{
  auto format = fmt();
  formatData->format = kRgbaSurfaceFormat;
  formatData->bitsPerPixel = format->bits_per_pixel;
  formatData->redShift = format->Rshift;
  formatData->greenShift = format->Gshift;
  formatData->blueShift = format->Bshift;
  formatData->alphaShift = format->Ashift;
  formatData->redMask = format->Rmask;
  formatData->greenMask = format->Gmask;
  formatData->blueMask = format->Bmask;
  formatData->alphaMask = format->Amask;
}

gfx::Color SDL3Surface::getPixel(int x, int y) const
{
  auto format = fmt();
  unsigned int data = 0;
  if (format->bytes_per_pixel == 4)
    data = *reinterpret_cast<uint32_t*>(getData(x, y));
  else if (format->bytes_per_pixel == 2)
    data = *reinterpret_cast<uint16_t*>(getData(x, y));
  else if (format->bytes_per_pixel == 1)
    data = *reinterpret_cast<uint8_t*>(getData(x, y));
  return from_sdl(format, data);
}

void SDL3Surface::putPixel(gfx::Color color, int x, int y)
{
  SDL_Rect clip;
  SDL_GetSurfaceClipRect(m_bmp, &clip);
  if (x < clip.x || x >= clip.x + clip.w || y < clip.y || y >= clip.y + clip.h)
    return;
  auto format = fmt();
  const unsigned int sdlColor = to_sdl(format, color);
  if (format->bytes_per_pixel == 4)
    *reinterpret_cast<uint32_t*>(getData(x, y)) = sdlColor;
  else if (format->bytes_per_pixel == 2)
    *reinterpret_cast<uint16_t*>(getData(x, y)) = sdlColor;
  else if (format->bytes_per_pixel == 1)
    *reinterpret_cast<uint8_t*>(getData(x, y)) = sdlColor;
}

void SDL3Surface::drawHLine(gfx::Color color, int x, int y, int w)
{
  // Our lock()/unlock() are the only source of SDL_LockSurface() calls on
  // m_bmp, so m_lock > 0 is equivalent to SDL2's old m_bmp->locked check.
  const bool dlocked = m_lock > 0;
  if (dlocked)
    SDL_UnlockSurface(m_bmp);

  SDL_Rect clip;
  SDL_GetSurfaceClipRect(m_bmp, &clip);

  if (x < clip.x)
  {
    w += x - clip.x;
    x = clip.x;
  }

  if (x + w > clip.x + clip.w)
  {
    w = (clip.x + clip.w) - x;
  }

  if (w <= 0 || y < clip.y || y >= (clip.y + clip.h))
  {
    if (dlocked)
      SDL_LockSurface(m_bmp);
    return;
  }

  auto format = fmt();
  const unsigned int sdlColor =
      to_sdl(format,
             gfx::rgba(gfx::getr(color), gfx::getg(color), gfx::getb(color)));
  auto data = getData(x, y);
  switch (drawMode)
  {
  case she::DrawMode::Solid:
    if (format->bytes_per_pixel == 4)
    {
      const int a = gfx::geta(color);
      const int ia = 255 - a;
      const int sr = gfx::getr(color) * a >> 8;
      const int sg = gfx::getg(color) * a >> 8;
      const int sb = gfx::getb(color) * a >> 8;
      for (; w--; data += 4)
      {
        const int r = (data[0] * ia >> 8) + sr;
        const int g = (data[1] * ia >> 8) + sg;
        const int b = (data[2] * ia >> 8) + sb;
        *reinterpret_cast<uint32_t*>(data) =
            (r) | (g << 8) | (b << 16) | (data[3] << 24);
      }
    }
    else
    {
      clip = {x, y, w, 1};
      SDL_FillSurfaceRect(m_bmp, &clip, sdlColor);
    }
    break;

  case she::DrawMode::Checked:
  {
    int offset = checkedModeOffset + x + y * 4;
    if (format->bytes_per_pixel == 4)
    {
      for (; w--; data += 4)
        *reinterpret_cast<uint32_t*>(data) =
            ((++offset) & 7) < 4 ? 0xFFFFFFFF : 0xFF000000;
    }
    else if (format->bytes_per_pixel == 2)
    {
      for (; w--; data += 2)
        *reinterpret_cast<uint16_t*>(data) = sdlColor;
    }
    else if (format->bytes_per_pixel == 1)
    {
      for (; w--; data += 1)
        *reinterpret_cast<uint8_t*>(data) = sdlColor;
    }
    break;
  }

  case she::DrawMode::Xor:
    if (format->bytes_per_pixel == 4)
    {
      for (; w--; data += 4)
        *reinterpret_cast<uint32_t*>(data) ^= sdlColor;
    }
    else if (format->bytes_per_pixel == 2)
    {
      for (; w--; data += 2)
        *reinterpret_cast<uint16_t*>(data) ^= sdlColor;
    }
    else if (format->bytes_per_pixel == 1)
    {
      for (; w--; data += 1)
        *reinterpret_cast<uint8_t*>(data) ^= sdlColor;
    }
    break;
  }

  if (dlocked)
    SDL_LockSurface(m_bmp);
}

void SDL3Surface::drawVLine(gfx::Color color, int x, int y, int h)
{
  SDL_Rect clip;
  SDL_GetSurfaceClipRect(m_bmp, &clip);

  if (y < clip.y)
  {
    h += y - clip.y;
    y = clip.y;
  }
  if (y + h > clip.y + clip.h)
  {
    h = (clip.y + clip.h) - y;
  }
  if (h <= 0 || x < clip.x || x >= clip.x + clip.w)
  {
    return;
  }
  auto format = fmt();
  const unsigned int sdlColor = to_sdl(format, color);
  auto data = getData(x, y);
  auto stride = m_bmp->pitch;
  switch (drawMode)
  {
  case she::DrawMode::Solid:
    if (format->bytes_per_pixel == 4)
    {
      const int a = gfx::geta(color);
      const int ia = 255 - a;
      const int sr = gfx::getr(color) * a >> 8;
      const int sg = gfx::getg(color) * a >> 8;
      const int sb = gfx::getb(color) * a >> 8;
      for (; h--; data += stride)
      {
        const int r = (data[0] * ia >> 8) + sr;
        const int g = (data[1] * ia >> 8) + sg;
        const int b = (data[2] * ia >> 8) + sb;
        *reinterpret_cast<uint32_t*>(data) =
            (r) | (g << 8) | (b << 16) | (data[3] << 24);
      }
    }
    else
    {
      clip = {x, y, 1, h};
      SDL_FillSurfaceRect(m_bmp, &clip, sdlColor);
    }
    break;

  case she::DrawMode::Checked:
  {
    int offset = checkedModeOffset + x + y;
    if (format->bytes_per_pixel == 4)
    {
      for (; h--; data += stride)
        *reinterpret_cast<uint32_t*>(data) =
            ((++offset) & 7) < 4 ? 0xFFFFFFFF : 0xFF000000;
    }
    else if (format->bytes_per_pixel == 2)
    {
      for (; h--; data += stride)
        *reinterpret_cast<uint16_t*>(data) = sdlColor;
    }
    else if (format->bytes_per_pixel == 1)
    {
      for (; h--; data += stride)
        *reinterpret_cast<uint8_t*>(data) = sdlColor;
    }
    break;
  }

  case she::DrawMode::Xor:
    if (format->bytes_per_pixel == 4)
    {
      for (; h--; data += stride)
        *reinterpret_cast<uint32_t*>(data) ^= sdlColor;
    }
    else if (format->bytes_per_pixel == 2)
    {
      for (; h--; data += stride)
        *reinterpret_cast<uint16_t*>(data) ^= sdlColor;
    }
    else if (format->bytes_per_pixel == 1)
    {
      for (; h--; data += stride)
        *reinterpret_cast<uint8_t*>(data) ^= sdlColor;
    }
    break;
  }
}

void SDL3Surface::drawLine(gfx::Color color, const gfx::Point& a,
                           const gfx::Point& b)
{
  if (a.x == b.x)
  {
    int y = a.y;
    int h = b.y - a.y;
    if (h < 0)
    {
      y = b.y;
      h = -h;
    }
    drawVLine(color, a.x, y, h);
    return;
  }

  if (a.y == b.y)
  {
    int x = a.x;
    int w = b.x - a.x;
    if (w < 0)
    {
      x = b.x;
      w = -w;
    }
    drawHLine(color, x, a.y, w);
    return;
  }

  // General case: Bresenham's line algorithm. putPixel() already clips
  // against the surface's clip rect, so no bounds checking is needed here.
  const int dx = std::abs(b.x - a.x), sx = (a.x < b.x) ? 1 : -1;
  const int dy = -std::abs(b.y - a.y), sy = (a.y < b.y) ? 1 : -1;
  int err = dx + dy;
  int x = a.x, y = a.y;
  while (true)
  {
    putPixel(color, x, y);
    if (x == b.x && y == b.y)
      break;
    const int e2 = 2 * err;
    if (e2 >= dy)
    {
      err += dy;
      x += sx;
    }
    if (e2 <= dx)
    {
      err += dx;
      y += sy;
    }
  }
}

void SDL3Surface::drawRect(gfx::Color color, const gfx::Rect& rc)
{
  drawHLine(color, rc.x, rc.y, rc.w - 1);
  drawHLine(color, rc.x, rc.y + rc.h - 1, rc.w - 1);
  drawVLine(color, rc.x, rc.y, rc.h - 1);
  drawVLine(color, rc.x + rc.w - 1, rc.y, rc.h);
}

void SDL3Surface::fillRect(gfx::Color color, const gfx::Rect& rc)
{
  auto alpha = gfx::geta(color);
  if (!alpha)
    return;
  if (alpha != 255)
  {
    if (!sdl::tempSurface)
      sdl::tempSurface = new SDL3Surface(1, 1, SDL3Surface::DeleteAndDestroy);
    SDL_FillSurfaceRect(sdl::tempSurface->m_bmp, nullptr,
                        to_sdl(sdl::tempSurface->fmt(), color));
    SDL_Rect rect{.x = rc.x, .y = rc.y, .w = rc.w, .h = rc.h};
    const SDL_Rect srcRect{.x = 0, .y = 0, .w = 1, .h = 1};
    SDL_BlitSurfaceScaled(sdl::tempSurface->m_bmp, &srcRect, m_bmp, &rect,
                          SDL_SCALEMODE_NEAREST);
  }
  else
  {
    const SDL_Rect rect{.x = rc.x, .y = rc.y, .w = rc.w, .h = rc.h};
    SDL_FillSurfaceRect(m_bmp, &rect, to_sdl(fmt(), color));
  }
}

void SDL3Surface::blitTo(Surface* dest, int srcx, int srcy, int dstx, int dsty,
                         int width, int height) const
{
  ASSERT(m_bmp);
  ASSERT(dest);
  // cppcheck-suppress nullPointerRedundantCheck
  ASSERT(static_cast<SDL3Surface*>(dest)->m_bmp);
  if (!m_bmp || !dest || !static_cast<SDL3Surface*>(dest)->m_bmp)
    return;

  const SDL_Rect srect{.x = srcx, .y = srcy, .w = width, .h = height};
  SDL_Rect drect{.x = dstx, .y = dsty};

  auto srcbmp = static_cast<const SDL3Surface*>(dest)->m_bmp;
  const bool slocked = static_cast<const SDL3Surface*>(dest)->m_lock > 0;
  const bool dlocked = m_lock > 0;
  if (slocked)
    SDL_UnlockSurface(srcbmp);
  if (dlocked)
    SDL_UnlockSurface(m_bmp);
  SDL_BlitSurface(m_bmp, &srect, srcbmp, &drect);
  if (slocked)
    SDL_LockSurface(srcbmp);
  if (dlocked)
    SDL_LockSurface(m_bmp);
}

void SDL3Surface::scrollTo(const gfx::Rect& rc, int dx, int dy)
{
  int w = width();
  int h = height();
  gfx::Clip clip(rc.x + dx, rc.y + dy, rc);
  if (!clip.clip(w, h, w, h))
    return;

  const int bytesPerPixel = fmt()->bytes_per_pixel;
  const int rowBytes = static_cast<int>(m_bmp->pitch);
  int rowDelta;

  if (dy > 0)
  {
    clip.src.y += clip.size.h - 1;
    clip.dst.y += clip.size.h - 1;
    rowDelta = -rowBytes;
  }
  else
    rowDelta = rowBytes;

  char* dst = reinterpret_cast<char*>(m_bmp->pixels);
  const char* src = dst;
  dst += static_cast<ptrdiff_t>(rowBytes) * clip.dst.y +
         static_cast<ptrdiff_t>(bytesPerPixel) * clip.dst.x;
  src += static_cast<ptrdiff_t>(rowBytes) * clip.src.y +
         static_cast<ptrdiff_t>(bytesPerPixel) * clip.src.x;
  w = bytesPerPixel * clip.size.w;
  h = clip.size.h;

  while (--h >= 0)
  {
    memmove(dst, src, w);
    dst += rowDelta;
    src += rowDelta;
  }
}

void SDL3Surface::drawSurface(const Surface* src, int dstx, int dsty)
{
  src->blitTo(this, 0, 0, dstx, dsty, src->width(), src->height());
}

void SDL3Surface::drawRgbaSurface(const Surface* src, int dstx, int dsty)
{
  src->blitTo(this, 0, 0, dstx, dsty, src->width(), src->height());
}

SDL_Texture* SDL3Surface::getTexture(const SDL_Rect* rect)
{
  auto format = fmt();
  const int x = rect ? rect->x : 0;
  const int y = rect ? rect->y : 0;
  auto* pixels = reinterpret_cast<uint8_t*>(m_bmp->pixels) +
                 static_cast<ptrdiff_t>(m_bmp->pitch) * y +
                 static_cast<ptrdiff_t>(format->bytes_per_pixel) * x;
  if (m_texture && m_textureGen != textureGen)
  {
    SDL_DestroyTexture(m_texture);
    m_texture = nullptr;
  }
  if (!m_texture)
  {
    m_textureGen = textureGen;
    auto renderer = she::unique_display->renderer();
    m_texture =
        SDL_CreateTexture(renderer, m_bmp->format,
                          SDL_TEXTUREACCESS_STREAMING, width(), height());
    // SDL3 textures default to linear filtering; SDL2 was nearest. Force
    // nearest to keep the canvas/UI pixel-crisp at scale >= 2 (see #261).
    SDL_SetTextureScaleMode(m_texture, SDL_SCALEMODE_NEAREST);
    SDL_UpdateTexture(m_texture, nullptr, pixels, m_bmp->pitch);
  }
  else
  {
    SDL_UpdateTexture(m_texture, rect, pixels, m_bmp->pitch);
  }
  return m_texture;
}
} // namespace she
