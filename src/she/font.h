// SHE Library
// Aseprite    | Copyright (C) 2012-2016 David Capello
// LibreSprite | Copyright (C) 2021      LibreSprite contributors
// Besprited   | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include "gfx/rect.h"

#include <cstdint>
#include <string>

namespace she
{

enum class FontType : std::uint8_t
{
  kUnknown,
  kSpriteSheet,
  kTrueType,
};

class Font
{
public:
  virtual ~Font() = default;
  virtual FontType type() = 0;
  [[nodiscard]] virtual int height() const = 0;
  [[nodiscard]] virtual gfx::Rect charBounds(int chr) const = 0;
  [[nodiscard]] virtual int charWidth(int chr) const = 0;
  [[nodiscard]] virtual int textLength(const std::string& str) const = 0;
  [[nodiscard]] virtual bool isScalable() const = 0;
  virtual void setSize(int size) = 0;
  virtual void setAntialias(bool antialias) = 0;
};

} // namespace she
