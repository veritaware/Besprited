// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "app/cmd.h"
#include "app/cmd/with_layer.h"

#include <string>

namespace app::cmd
{
using namespace doc;

class SetLayerName : public Cmd,
                     public WithLayer
{
public:
  SetLayerName(const Layer* layer, std::string name);

protected:
  void onExecute() override;
  void onUndo() override;
  void onFireNotifications() override;
  [[nodiscard]] size_t onMemSize() const override { return sizeof(*this); }

private:
  std::string m_oldName;
  std::string m_newName;
};

} // namespace app::cmd
