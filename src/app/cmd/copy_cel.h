// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "app/cmd/with_layer.h"
#include "app/cmd_sequence.h"
#include "doc/frame.h"

namespace doc
{
class LayerImage;
}

namespace app::cmd
{
using namespace doc;

class CopyCel : public CmdSequence
{
public:
  CopyCel(const LayerImage* srcLayer, frame_t srcFrame,
          const LayerImage* dstLayer, frame_t dstFrame, bool continuous);

protected:
  void onExecute() override;
  void onFireNotifications() override;

private:
  WithLayer m_srcLayer, m_dstLayer;
  frame_t m_srcFrame, m_dstFrame;
  bool m_continuous;
};

} // namespace app::cmd