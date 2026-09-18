// Aseprite  | Copyright (C) 2001-2016 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "app/cmd.h"
#include "app/cmd/with_cel.h"
#include "app/cmd/with_layer.h"

#include <sstream>

namespace doc
{
class Cel;
class Layer;
}

namespace app::cmd
{
using namespace doc;

class AddCel : public Cmd,
               public WithLayer,
               public WithCel
{
public:
  AddCel(const Layer* layer, const std::shared_ptr<Cel>& cel);

protected:
  void onExecute() override;
  void onUndo() override;
  void onRedo() override;
  size_t onMemSize() const override { return sizeof(*this) + m_size; }

private:
  static void addCel(Layer* layer, const std::shared_ptr<Cel>& cel);
  static void removeCel(Layer* layer, const std::shared_ptr<Cel>& cel);

  size_t m_size;
  std::stringstream m_stream;
};
} // namespace app::cmd
