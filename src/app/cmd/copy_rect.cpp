// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/cmd/copy_rect.h"

#include "doc/image.h"

#include <algorithm>
#include <utility>

namespace app::cmd
{

CopyRect::CopyRect(const Image* dst, const Image* src, gfx::Clip clip)
  : WithImage(dst)
  , m_clip(std::move(clip))
{
  if (!m_clip.clip(dst->width(), dst->height(), src->width(), src->height()))
    return;

  // Fill m_data with "src" data

  const int lineSize = src->getRowStrideSize(m_clip.size.w);
  m_data.resize(static_cast<long>(lineSize) * m_clip.size.h);

  auto it = m_data.begin();
  for (int v = 0; v < m_clip.size.h; ++v)
  {
    uint8_t* addr = src->getPixelAddress(m_clip.dst.x, m_clip.dst.y + v);

    std::copy_n(addr, lineSize, it);
    it += lineSize;
  }
}

void CopyRect::onExecute()
{
  swap();
}

void CopyRect::onUndo()
{
  swap();
}

void CopyRect::onRedo()
{
  swap();
}

void CopyRect::swap()
{
  if (m_clip.size.w < 1 || m_clip.size.h < 1)
    return;

  Image* image = this->image();
  const int lineSize = this->lineSize();
  std::vector<uint8_t> tmp(lineSize);

  auto it = m_data.begin();
  for (int v = 0; v < m_clip.size.h; ++v)
  {
    uint8_t* addr = image->getPixelAddress(m_clip.dst.x, m_clip.dst.y + v);

    std::copy_n(addr, lineSize, tmp.begin());
    std::copy_n(it, lineSize, addr);
    std::ranges::copy(tmp, it);

    it += lineSize;
  }

  image->incrementVersion();
}

int CopyRect::lineSize() const
{
  return image()->getRowStrideSize(m_clip.size.w);
}

} // namespace app::cmd
