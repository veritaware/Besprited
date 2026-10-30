// SHE library
// Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include "clip/clip.h"
#include "she/clipboard.h"

namespace she
{

// Deprecated: clip-based implementation, removed together with SDL2 (#274).
class SDL2Clipboard : public Clipboard
{
public:
  SDL2Clipboard();

  bool getText(std::string& text) override;
  bool setText(const std::string& text) override;
  bool hasImage() override;
  bool getImageSize(gfx::Size& size) override;
  bool getImage(ClipboardImage& image) override;
  bool hasCustomData() override;
  bool getCustomData(std::vector<uint8_t>& data) override;
  bool setImage(const ClipboardImage& image, const std::vector<uint8_t>* customData) override;

private:
  clip::format m_customFormat;
};

} // namespace she
