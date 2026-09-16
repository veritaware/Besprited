// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/cmd/shift_masked_cel.h"

#include "app/document.h"
#include "doc/algorithm/shift_image.h"
#include "doc/cel.h"
#include "doc/image.h"
#include "doc/layer.h"
#include "doc/mask.h"

namespace app::cmd
{

ShiftMaskedCel::ShiftMaskedCel(const std::shared_ptr<Cel>& cel, const int dx,
                               const int dy)
  : WithCel(cel)
  , m_dx(dx)
  , m_dy(dy)
{
}

void ShiftMaskedCel::onExecute()
{
  shift(m_dx, m_dy);
}

void ShiftMaskedCel::onUndo()
{
  shift(-m_dx, -m_dy);
}

void ShiftMaskedCel::shift(const int dx, const int dy) const
{
  const auto cel = this->cel();
  Image* image = cel->image();
  Mask* mask = dynamic_cast<Document*>(cel->document())->mask();
  ASSERT(mask->bitmap());
  if (!mask->bitmap())
    return;

  const int x = cel->x();
  const int y = cel->y();

  mask->offsetOrigin(-x, -y);
  algorithm::shift_image_with_mask(image, mask, dx, dy);
  mask->offsetOrigin(x, y);

  image->incrementVersion();
}

} // namespace app::cmd
