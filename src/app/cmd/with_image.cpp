// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/cmd/with_image.h"

#include "doc/image.h"

namespace app::cmd
{

using namespace doc;

WithImage::WithImage(const Image* image)
  : m_imageId(image->id())
{
}

Image* WithImage::image() const
{
  return get<Image>(m_imageId);
}

} // namespace app::cmd
