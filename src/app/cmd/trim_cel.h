// Aseprite  | Copyright (C) 2016 David Capello
// Besprited | Copyright (C) 2026 Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "app/cmd.h"

namespace doc
{
class Cel;
}

namespace app::cmd
{

class TrimCel : public Cmd
{
public:
  explicit TrimCel(const std::shared_ptr<doc::Cel>& cel);
  ~TrimCel() override;

protected:
  void onExecute() override;
  void onUndo() override;
  void onRedo() override;
  [[nodiscard]] size_t onMemSize() const override
  {
    return sizeof(*this) + m_subCmd->memSize();
  }

private:
  Cmd* m_subCmd;
};

} // namespace app::cmd
