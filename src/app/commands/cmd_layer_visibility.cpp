// Aseprite  | Copyright (C) 2001-2016 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/commands/command.h"
#include "app/context_access.h"
#include "app/modules/gui.h"
#include "doc/image.h"
#include "doc/layer.h"

namespace app
{

using namespace ui;

class LayerVisibilityCommand : public Command
{
public:
  LayerVisibilityCommand();

protected:
  bool onEnabled(Context* context) override;
  bool onChecked(Context* context) override;
  void onExecute(Context* context) override;
};

LayerVisibilityCommand::LayerVisibilityCommand()
  : Command("LayerVisibility", "Layer Visibility", CmdRecordableFlag)
{
}

bool LayerVisibilityCommand::onEnabled(Context* context)
{
  return context->checkFlags(ContextFlags::ActiveDocumentIsWritable |
                             ContextFlags::HasActiveLayer);
}

bool LayerVisibilityCommand::onChecked(Context* context)
{
  const ContextReader reader(context);
  const Layer* layer = reader.layer();
  return (layer && layer->isVisible());
}

void LayerVisibilityCommand::onExecute(Context* context)
{
  ContextWriter writer(context);
  Layer* layer = writer.layer();

  layer->setVisible(!layer->isVisible());

  update_screen_for_document(writer.document());
}

std::unique_ptr<Command> CommandFactory::createLayerVisibilityCommand()
{
  return std::make_unique<LayerVisibilityCommand>();
}

} // namespace app
