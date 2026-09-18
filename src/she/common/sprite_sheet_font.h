// SHE Library
// Aseprite    | Copyright (C) 2012-2016 David Capello
// LibreSprite | Copyright (C) 2021      LibreSprite contributors
// Besprited   | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include "base/debug.h"
#include "base/string.h"
#include "gfx/rect.h"
#include "she/font.h"
#include "she/surface.h"

#include <vector>

namespace she
{

class SpriteSheetFont : public Font
{
public:
  SpriteSheetFont() = default;

  ~SpriteSheetFont()
  {
    ASSERT(m_sheet);
    if (m_sheet)
      m_sheet->dispose();
  }

  FontType type() override { return FontType::kSpriteSheet; }

  [[nodiscard]] int height() const override { return getCharBounds(' ').h; }

  [[nodiscard]] gfx::Rect charBounds(int chr) const override
  {
    return getCharBounds(chr);
  }

  [[nodiscard]] int charWidth(int chr) const override
  {
    return charBounds(chr).w;
  }

  [[nodiscard]] int textLength(const std::string& str) const override
  {
    base::utf8_const_iterator it(str.begin()), end(str.end());
    int x = 0;
    while (it != end)
    {
      x += charWidth(*it);
      ++it;
    }
    return x;
  }

  [[nodiscard]] bool isScalable() const override { return false; }

  void setSize(int size) override
  {
    // Do nothing
  }

  void setAntialias(bool antialias) override
  {
    // Do nothing
  }

  [[nodiscard]] Surface* getSurfaceSheet() const { return m_sheet; }

  [[nodiscard]] gfx::Rect getCharBounds(int chr) const
  {
    chr -= static_cast<int>(' ');
    if (chr >= 0 && chr < static_cast<int>(m_chars.size()))
      return m_chars[chr];
    else
      return {};
  }

  static Font* fromSurface(Surface* sur)
  {
    auto* font = new SpriteSheetFont;
    font->m_sheet = sur;

    const SurfaceLock lock(sur);
    gfx::Rect bounds(0, 0, 1, 1);

    while (font->findChar(sur, sur->width(), sur->height(), bounds))
    {
      font->m_chars.push_back(bounds);
      bounds.x += bounds.w;
    }

    return font;
  }

private:
  bool findChar(const Surface* sur, int width, int height, gfx::Rect& bounds)
  {
    const gfx::Color keyColor = sur->getPixel(0, 0);

    while (sur->getPixel(bounds.x, bounds.y) == keyColor)
    {
      bounds.x++;
      if (bounds.x >= width)
      {
        bounds.x = 0;
        bounds.y += bounds.h;
        bounds.h = 1;
        if (bounds.y >= height)
          return false;
      }
    }

    bounds.w = 0;
    while ((bounds.x + bounds.w < width) &&
           (sur->getPixel(bounds.x + bounds.w, bounds.y) != keyColor))
    {
      bounds.w++;
    }

    bounds.h = 0;
    while ((bounds.y + bounds.h < height) &&
           (sur->getPixel(bounds.x, bounds.y + bounds.h) != keyColor))
    {
      bounds.h++;
    }

    return !bounds.isEmpty();
  }

private:
  Surface* m_sheet = nullptr;
  std::vector<gfx::Rect> m_chars;
};

} // namespace she
