// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "app/cmd.h"
#include "app/cmd/with_image.h"
#include "app/cmd_sequence.h"
#include "doc/image.h"

#include <memory>

namespace doc
{
class Cel;
}

namespace app::cmd
{
using namespace doc;

class ClearRect : public Cmd
{
public:
  ClearRect(const std::shared_ptr<Cel>& cel, const gfx::Rect& bounds);

protected:
  void onExecute() override;
  void onUndo() override;
  void onRedo() override;
  [[nodiscard]] size_t onMemSize() const override
  {
    return sizeof(*this) + m_seq.memSize() +
           (m_copy ? m_copy->getMemSize() : 0);
  }

private:
  void clear() const;
  void restore() const;

  CmdSequence m_seq;
  std::unique_ptr<WithImage> m_dstImage;
  std::shared_ptr<Image> m_copy;
  int m_offsetX, m_offsetY;
  color_t m_bgcolor;
};

} // namespace app::cmd
