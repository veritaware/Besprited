// Aseprite  | Copyright (C) 2016 David Capello
// Besprited | Copyright (C) 2026 Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/cmd/patch_cel.h"

#include "app/cmd/copy_region.h"
#include "app/cmd/crop_cel.h"
#include "app/cmd/trim_cel.h"
#include "doc/cel.h"

namespace app::cmd
{

using namespace doc;

PatchCel::PatchCel(const std::shared_ptr<Cel>& dstCel, const Image* patch,
                   const gfx::Region& patchedRegion, const gfx::Point& patchPos)
  : WithCel(dstCel)
  , m_patch(patch)
  , m_region(patchedRegion)
  , m_pos(patchPos)
{
}

void PatchCel::onExecute()
{
  const auto cel = this->cel();

  executeAndAdd(new CropCel(
      cel, cel->bounds() | gfx::Rect(m_region.bounds()).offset(m_pos)));

  executeAndAdd(
      new CopyRegion(cel->image(), m_patch, m_region, m_pos - cel->position()));

  executeAndAdd(new TrimCel(cel));

  m_patch = nullptr;
}

} // namespace app::cmd
