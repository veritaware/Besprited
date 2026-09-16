// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "app/cmd.h"
#include "app/cmd/with_frame_tag.h"
#include "doc/frame.h"

namespace app::cmd
{
using namespace doc;

class SetFrameTagRange : public Cmd,
                         public WithFrameTag
{
public:
  SetFrameTagRange(const FrameTag* tag, frame_t from, frame_t to);

protected:
  void onExecute() override;
  void onUndo() override;
  [[nodiscard]] size_t onMemSize() const override { return sizeof(*this); }

private:
  frame_t m_oldFrom, m_oldTo;
  frame_t m_newFrom, m_newTo;
};

} // namespace app::cmd
