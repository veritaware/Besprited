// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "app/cmd.h"
#include "app/cmd/with_layer.h"
#include "doc/layer.h"

namespace app::cmd
{
using namespace doc;

class SetLayerFlags : public Cmd,
                      public WithLayer
{
public:
  SetLayerFlags(const Layer* layer, LayerFlags flags);

protected:
  void onExecute() override;
  void onUndo() override;
  [[nodiscard]] size_t onMemSize() const override { return sizeof(*this); }

private:
  LayerFlags m_oldFlags;
  LayerFlags m_newFlags;
};

} // namespace app::cmd
