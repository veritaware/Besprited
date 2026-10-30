// SHE library
// Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include "she/clipboard.h"

namespace she
{

class SDL3Clipboard : public Clipboard
{
public:
  bool getText(std::string& text) override;
  bool setText(const std::string& text) override;
  bool hasImage() override;
  bool getImageSize(gfx::Size& size) override;
  bool getImage(ClipboardImage& image) override;
  bool hasCustomData() override;
  bool getCustomData(std::vector<uint8_t>& data) override;
  bool setImage(const ClipboardImage& image, const std::vector<uint8_t>* customData) override;
};

} // namespace she
