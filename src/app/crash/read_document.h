// Aseprite  | Copyright (C) 2001-2016 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "app/crash/raw_images_as.h"
#include "doc/frame.h"
#include "doc/pixel_format.h"

#include <string>

namespace app
{
class Document;
namespace crash
{

struct DocumentInfo
{
  doc::PixelFormat format = doc::IMAGE_RGB;
  int width = 0;
  int height = 0;
  doc::frame_t frames = 0;
  std::string filename;
};

bool read_document_info(const std::string& dir, DocumentInfo& info);
app::Document* read_document(const std::string& dir);
app::Document* read_document_with_raw_images(const std::string& dir,
                                             RawImagesAs as);

} // namespace crash
} // namespace app
