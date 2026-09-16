// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "app/cmd.h"
#include "app/cmd/with_frame_tag.h"
#include "doc/anidir.h"

namespace app::cmd
{
using namespace doc;

class SetFrameTagAniDir : public Cmd,
                          public WithFrameTag
{
public:
  SetFrameTagAniDir(const FrameTag* tag, AniDir anidir);

protected:
  void onExecute() override;
  void onUndo() override;
  [[nodiscard]] size_t onMemSize() const override { return sizeof(*this); }

private:
  AniDir m_oldAniDir;
  AniDir m_newAniDir;
};

} // namespace app::cmd
