// LibreSprite | Copyright (C) 2026 LibreSprite contributors
// Besprited   | Copyright (C) 2026 Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/cmd/set_pixel.h"

#include "base/debug.h"
#include "base/arithmetic.h"
#include "doc/image.h"

namespace app
{
namespace cmd
{

SetPixel::SetPixel(Image* image, int x, int y, color_t color)
  : WithImage(image)
{
  ASSERT(base::in_range(x, 0, image->width(), base::RangeBound::RightOpen));
  ASSERT(base::in_range(y, 0, image->height(), base::RangeBound::RightOpen));
  m_pos = gfx::Point(x, y);
  m_oldColor = image->getPixel(x, y);
  m_newColor = color;
}

void SetPixel::putPixel(color_t color)
{
  this->image()->putPixel(m_pos.x, m_pos.y, color);
  this->image()->incrementVersion();
}

void SetPixel::onExecute()
{
  putPixel(m_newColor);
}

void SetPixel::onUndo()
{
  putPixel(m_oldColor);
}

void SetPixel::onRedo()
{
  putPixel(m_newColor);
}

} // namespace cmd
} // namespace app
