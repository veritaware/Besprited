// Aseprite  | Copyright (C) 2001-2016 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "app/cmd.h"
#include "app/cmd/with_layer.h"

#include <sstream>

namespace doc
{
class Layer;
}

namespace app::cmd
{
using namespace doc;

class AddLayer : public Cmd
{
public:
  AddLayer(const Layer* folder, const Layer* newLayer, const Layer* afterThis);

protected:
  void onExecute() override;
  void onUndo() override;
  void onRedo() override;
  size_t onMemSize() const override { return sizeof(*this) + m_size; }

private:
  static void addLayer(Layer* folder, Layer* newLayer, Layer* afterThis);
  static void removeLayer(Layer* folder, Layer* layer);

  WithLayer m_folder;
  WithLayer m_newLayer;
  WithLayer m_afterThis;
  size_t m_size;
  std::stringstream m_stream;
};
} // namespace app::cmd
