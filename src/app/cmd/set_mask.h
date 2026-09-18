// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "app/cmd.h"
#include "app/cmd/with_document.h"

#include <memory>

namespace doc
{
class Mask;
}

namespace app::cmd
{
using namespace doc;

class SetMask : public Cmd,
                public WithDocument
{
public:
  SetMask(const Document* doc, const Mask* newMask);

  // Used to change the new mask used in the onRedo()
  void setNewMask(const Mask* newMask);

protected:
  void onExecute() override;
  void onUndo() override;
  [[nodiscard]] size_t onMemSize() const override;

private:
  void setMask(const Mask* mask) const;

  std::unique_ptr<Mask> m_oldMask;
  std::unique_ptr<Mask> m_newMask;
};

} // namespace app::cmd
