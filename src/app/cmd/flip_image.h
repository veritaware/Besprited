// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "app/cmd.h"
#include "app/cmd/with_image.h"
#include "doc/algorithm/flip_type.h"
#include "gfx/rect.h"

#include <vector>

namespace doc
{
class Image;
}

namespace app::cmd
{
using namespace doc;

class FlipImage : public Cmd,
                  public WithImage
{
public:
  FlipImage(const Image* image, const gfx::Rect& bounds,
            algorithm::FlipType flipType);

protected:
  void onExecute() override;
  void onUndo() override;
  [[nodiscard]] size_t onMemSize() const override { return sizeof(*this); }

private:
  void swap() const;

  gfx::Rect m_bounds;
  algorithm::FlipType m_flipType;
};

} // namespace app::cmd
