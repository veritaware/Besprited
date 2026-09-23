// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/app.h"
#include "app/commands/command.h"
#include "app/context_access.h"
#include "app/modules/editors.h"
#include "app/modules/gui.h"
#include "app/ui/editor/editor.h"
#include "app/ui/status_bar.h"
#include "doc/layer.h"
#include "doc/sprite.h"

namespace app
{

class GotoCommand : public Command
{
public:
  GotoCommand(const char* short_name, const char* friendly_name,
              CommandFlags flags)
    : Command(short_name, friendly_name, flags)
  {
  }

protected:
  void updateStatusBar(Site& site)
  {
    if (site.layer() != nullptr)
      StatusBar::instance()->setStatusText(1000, "Layer `%s' selected",
                                           site.layer()->name().c_str());
  }
};

class GotoPreviousLayerCommand : public GotoCommand
{
public:
  GotoPreviousLayerCommand();

protected:
  bool onEnabled(Context* context) override;
  void onExecute(Context* context) override;
};

GotoPreviousLayerCommand::GotoPreviousLayerCommand()
  : GotoCommand("GotoPreviousLayer", "Go to Previous Layer", CmdUIOnlyFlag)
{
}

bool GotoPreviousLayerCommand::onEnabled(Context* context)
{
  return (current_editor != nullptr && current_editor->document());
}

void GotoPreviousLayerCommand::onExecute(Context* context)
{
  Site site = current_editor->getSite();

  if (site.layerIndex() > 0)
    site.layerIndex(site.layerIndex().previous());
  else
    site.layerIndex(LayerIndex(site.sprite()->countLayers() - 1));

  // Flash the current layer
  current_editor->setLayer(site.layer());
  current_editor->flashCurrentLayer();

  updateStatusBar(site);
}

class GotoNextLayerCommand : public GotoCommand
{
public:
  GotoNextLayerCommand();

protected:
  bool onEnabled(Context* context) override;
  void onExecute(Context* context) override;
};

GotoNextLayerCommand::GotoNextLayerCommand()
  : GotoCommand("GotoNextLayer", "Go to Next Layer", CmdUIOnlyFlag)
{
}

bool GotoNextLayerCommand::onEnabled(Context* context)
{
  return (current_editor != nullptr && current_editor->document());
}

void GotoNextLayerCommand::onExecute(Context* context)
{
  Site site = current_editor->getSite();

  if (site.layerIndex() < site.sprite()->countLayers() - 1)
    site.layerIndex(site.layerIndex().next());
  else
    site.layerIndex(LayerIndex(0));

  // Flash the current layer
  current_editor->setLayer(site.layer());
  current_editor->flashCurrentLayer();

  updateStatusBar(site);
}

std::unique_ptr<Command> CommandFactory::createGotoPreviousLayerCommand()
{
  return std::make_unique<GotoPreviousLayerCommand>();
}

std::unique_ptr<Command> CommandFactory::createGotoNextLayerCommand()
{
  return std::make_unique<GotoNextLayerCommand>();
}

} // namespace app
