// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "app/cmd/add_layer.h"

namespace app::cmd
{
using namespace doc;

class RemoveLayer : public AddLayer
{
public:
  explicit RemoveLayer(const Layer* layer);

protected:
  void onExecute() override;
  void onUndo() override;
  void onRedo() override;
};

} // namespace app::cmd
