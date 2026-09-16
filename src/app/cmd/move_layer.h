// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "app/cmd.h"
#include "app/cmd/with_layer.h"

namespace app::cmd
{
using namespace doc;

class MoveLayer : public Cmd
{
public:
  MoveLayer(const Layer* layer, const Layer* afterThis);

protected:
  void onExecute() override;
  void onUndo() override;
  void onFireNotifications() override;
  [[nodiscard]] size_t onMemSize() const override { return sizeof(*this); }

private:
  WithLayer m_layer;
  WithLayer m_oldAfterThis;
  WithLayer m_newAfterThis;
};

} // namespace app::cmd
