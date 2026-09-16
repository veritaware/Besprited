// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "app/cmd.h"
#include "app/cmd/with_document.h"
#include "gfx/point.h"

namespace app::cmd
{
using namespace doc;

class SetMaskPosition : public Cmd,
                        public WithDocument
{
public:
  SetMaskPosition(const Document* doc, const gfx::Point& pos);

protected:
  void onExecute() override;
  void onUndo() override;
  [[nodiscard]] size_t onMemSize() const override { return sizeof(*this); }

private:
  void setMaskPosition(const gfx::Point& pos) const;

  gfx::Point m_oldPosition;
  gfx::Point m_newPosition;
};

} // namespace app::cmd
