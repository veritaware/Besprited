// Aseprite  | Copyright (C) 2001-2016 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "app/cmd/with_cel.h"
#include "app/cmd_sequence.h"
#include "gfx/fwd.h"
#include "gfx/point.h"

namespace doc
{
class Cel;
class Image;
}

namespace app::cmd
{

class PatchCel : public CmdSequence,
                 public WithCel
{
public:
  PatchCel(const std::shared_ptr<Cel>& dstCel, const Image* patch,
           const gfx::Region& patchedRegion, const gfx::Point& patchPos);

protected:
  void onExecute() override;

  const Image* m_patch;
  const gfx::Region& m_region;
  gfx::Point m_pos;
};

} // namespace app::cmd
