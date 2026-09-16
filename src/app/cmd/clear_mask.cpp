// Aseprite  | Copyright (C) 2001-2016 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/cmd/clear_mask.h"

#include <memory>

#include "app/cmd/clear_cel.h"
#include "app/document.h"
#include "doc/cel.h"
#include "doc/image_impl.h"
#include "doc/layer.h"
#include "doc/mask.h"
#include "doc/primitives.h"

namespace app::cmd
{

using namespace doc;

ClearMask::ClearMask(const std::shared_ptr<Cel>& cel)
  : WithCel(cel)
{
  const auto* doc = dynamic_cast<Document*>(cel->document());

  // If the mask is empty or is not visible then we have to clear the
  // entire image in the cel.
  if (!doc->isMaskVisible())
  {
    m_seq.add(new ClearCel(cel));
    return;
  }

  Image* image = cel ? cel->image() : nullptr;
  if (!image)
    return;

  const Mask* mask = doc->mask();
  m_offsetX = mask->bounds().x - cel->x();
  m_offsetY = mask->bounds().y - cel->y();

  const gfx::Rect bounds = image->bounds().createIntersection(
      gfx::Rect(m_offsetX, m_offsetY, mask->bounds().w, mask->bounds().h));
  if (bounds.isEmpty())
    return;

  m_dstImage = std::make_unique<WithImage>(image);
  m_bgcolor = doc->bgColor(cel->layer());
  m_boundsX = bounds.x;
  m_boundsY = bounds.y;

  m_copy.reset(
      crop_image(image, bounds.x, bounds.y, bounds.w, bounds.h, m_bgcolor));
}

void ClearMask::onExecute()
{
  m_seq.execute(context());
  if (m_dstImage)
    clear();
}

void ClearMask::onUndo()
{
  if (m_dstImage)
    restore();
  m_seq.undo();
}

void ClearMask::onRedo()
{
  m_seq.redo();
  if (m_dstImage)
    clear();
}

void ClearMask::clear() const
{
  const auto cel = this->cel();
  Image* image = m_dstImage->image();
  const auto* doc = dynamic_cast<Document*>(cel->document());
  Mask* mask = doc->mask();

  ASSERT(mask->bitmap());
  if (!mask->bitmap())
    return;

  const LockImageBits<BitmapTraits> maskBits(mask->bitmap());
  auto it = maskBits.begin();

  // Clear the masked zones
  for (int v = 0; v < mask->bounds().h; ++v)
  {
    for (int u = 0; u < mask->bounds().w; ++u, ++it)
    {
      ASSERT(it != maskBits.end());
      if (*it)
      {
        put_pixel(image, u + m_offsetX, v + m_offsetY, m_bgcolor);
      }
    }
  }

  ASSERT(it == maskBits.end());
}

void ClearMask::restore() const
{
  copy_image(m_dstImage->image(), m_copy.get(), m_boundsX, m_boundsY);
}

} // namespace app::cmd
