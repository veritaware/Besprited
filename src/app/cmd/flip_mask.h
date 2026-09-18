// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "app/cmd.h"
#include "app/cmd/with_document.h"
#include "doc/algorithm/flip_type.h"

namespace app::cmd
{
using namespace doc;

class FlipMask : public Cmd,
                 public WithDocument
{
public:
  FlipMask(const Document* doc, algorithm::FlipType flipType);

protected:
  void onExecute() override;
  void onUndo() override;
  [[nodiscard]] size_t onMemSize() const override { return sizeof(*this); }

private:
  void swap() const;

  algorithm::FlipType m_flipType;
};

} // namespace app::cmd
