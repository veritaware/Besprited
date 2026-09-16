// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "app/cmd.h"
#include "app/cmd/with_cel.h"

namespace app::cmd
{
using namespace doc;

class SetCelFrame : public Cmd,
                    public WithCel
{
public:
  SetCelFrame(const std::shared_ptr<Cel>& cel, frame_t frame);

protected:
  void onExecute() override;
  void onUndo() override;
  void onFireNotifications() override;
  [[nodiscard]] size_t onMemSize() const override { return sizeof(*this); }

private:
  frame_t m_oldFrame;
  frame_t m_newFrame;
};

} // namespace app::cmd
