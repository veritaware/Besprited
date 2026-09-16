// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "app/cmd_sequence.h"

namespace doc
{
class Layer;
}

namespace app::cmd
{
using namespace doc;

class LayerFromBackground : public CmdSequence
{
public:
  explicit LayerFromBackground(const Layer* layer);
};

} // namespace app::cmd
