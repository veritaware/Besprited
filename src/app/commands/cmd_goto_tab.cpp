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

#include "app/app.h"
#include "app/ui/workspace.h"

namespace app
{

class GotoNextTabCommand : public Command
{
public:
  GotoNextTabCommand();

protected:
  bool onEnabled(Context* context) override;
  void onExecute(Context* context) override;
};

GotoNextTabCommand::GotoNextTabCommand()
  : Command("GotoNextTab", "Go to Next Tab", CmdUIOnlyFlag)
{
}

bool GotoNextTabCommand::onEnabled(Context* context)
{
  return App::instance()->workspace()->canSelectOtherTab();
}

void GotoNextTabCommand::onExecute(Context* context)
{
  App::instance()->workspace()->selectNextTab();
}

class GotoPreviousTabCommand : public Command
{
public:
  GotoPreviousTabCommand();

protected:
  bool onEnabled(Context* context) override;
  void onExecute(Context* context) override;
};

GotoPreviousTabCommand::GotoPreviousTabCommand()
  : Command("GotoPreviousTab", "Go to Previous tab", CmdRecordableFlag)
{
}

bool GotoPreviousTabCommand::onEnabled(Context* context)
{
  return App::instance()->workspace()->canSelectOtherTab();
}

void GotoPreviousTabCommand::onExecute(Context* context)
{
  App::instance()->workspace()->selectPreviousTab();
}

std::unique_ptr<Command> CommandFactory::createGotoNextTabCommand()
{
  return std::make_unique<GotoNextTabCommand>();
}

std::unique_ptr<Command> CommandFactory::createGotoPreviousTabCommand()
{
  return std::make_unique<GotoPreviousTabCommand>();
}

} // namespace app
