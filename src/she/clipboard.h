// SHE library
// Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include "gfx/fwd.h"

#include <cstdint>
#include <string>
#include <vector>

namespace she
{

// Tightly packed non-premultiplied pixels in R,G,B,A byte order.
struct ClipboardImage
{
  int width = 0;
  int height = 0;
  std::vector<uint8_t> rgba;
};

class Clipboard
{
public:
  virtual ~Clipboard() = default;

  virtual bool getText(std::string& text) = 0;
  virtual bool setText(const std::string& text) = 0;

  virtual bool hasImage() = 0;
  virtual bool getImageSize(gfx::Size& size) = 0;
  virtual bool getImage(ClipboardImage& image) = 0;

  // Opaque application-defined blob (image + mask + palette) that is
  // stored alongside the image so nothing is lost on a round trip.
  virtual bool hasCustomData() = 0;
  virtual bool getCustomData(std::vector<uint8_t>& data) = 0;

  // Replaces the clipboard contents with the image and optional custom data.
  virtual bool setImage(const ClipboardImage& image, const std::vector<uint8_t>* customData) = 0;
};

} // namespace she
