// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "app/cmd/add_cel.h"

namespace app::cmd
{
using namespace doc;

class RemoveCel : public AddCel
{
public:
  explicit RemoveCel(const std::shared_ptr<Cel>& cel);

protected:
  void onExecute() override;
  void onUndo() override;
  void onRedo() override;
};

} // namespace app::cmd
