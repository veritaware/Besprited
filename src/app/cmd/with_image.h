// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "doc/object_id.h"

namespace doc
{
class Image;
}

namespace app::cmd
{
using namespace doc;

class WithImage
{
public:
  explicit WithImage(const Image* image);
  [[nodiscard]] Image* image() const;

private:
  ObjectId m_imageId;
};

} // namespace app::cmd
