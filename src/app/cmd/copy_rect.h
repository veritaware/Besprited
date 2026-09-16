// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "app/cmd.h"
#include "app/cmd/with_image.h"
#include "gfx/clip.h"

#include <vector>

namespace doc
{
class Image;
}

namespace app::cmd
{
using namespace doc;

class CopyRect : public Cmd,
                 public WithImage
{
public:
  CopyRect(const Image* dst, const Image* src, gfx::Clip clip);

protected:
  void onExecute() override;
  void onUndo() override;
  void onRedo() override;
  [[nodiscard]] size_t onMemSize() const override
  {
    return sizeof(*this) + m_data.size();
  }

private:
  void swap();
  [[nodiscard]] int lineSize() const;

  gfx::Clip m_clip;
  std::vector<uint8_t> m_data;
};

} // namespace app::cmd
