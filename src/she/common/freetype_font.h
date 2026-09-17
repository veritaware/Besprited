// SHE Library
// Aseprite    | Copyright (C) 2016 David Capello
// LibreSprite | Copyright (C) 2021 LibreSprite contributors
// Besprited   | Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include "ft/face.h"
#include "ft/lib.h"
#include "she/font.h"

namespace she
{
class Font;

class FreeTypeFont : public Font
{
public:
  FreeTypeFont(const char* filename, int height);

  [[nodiscard]] bool isValid() const;
  FontType type() override;
  [[nodiscard]] int height() const override;
  [[nodiscard]] gfx::Rect charBounds(int chr) const override;
  [[nodiscard]] int charWidth(int chr) const override;
  [[nodiscard]] int textLength(const std::string& str) const override;
  [[nodiscard]] bool isScalable() const override;
  void setSize(int size) override;
  void setAntialias(bool antialias) override;

  ft::Face& face() { return m_face; }

private:
  mutable ft::Lib m_ft;
  mutable ft::Face m_face;
};

FreeTypeFont* loadFreeTypeFont(const char* filename, int height);

} // namespace she
