// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/commands/command.h"
#include "app/context.h"
#include "app/context_access.h"
#include "app/document.h"
#include "app/launcher.h"

namespace app
{

class OpenInFolderCommand : public Command
{
public:
  OpenInFolderCommand();

protected:
  bool onEnabled(Context* context) override;
  void onExecute(Context* context) override;
};

OpenInFolderCommand::OpenInFolderCommand()
  : Command("OpenInFolder", "Open In Folder", CmdUIOnlyFlag)
{
}

bool OpenInFolderCommand::onEnabled(Context* context)
{
  const ContextReader reader(context);
  return reader.document() && reader.document()->isAssociatedToFile();
}

void OpenInFolderCommand::onExecute(Context* context)
{
  launcher::open_folder(context->activeDocument()->filename());
}

std::unique_ptr<Command> CommandFactory::createOpenInFolderCommand()
{
  return std::make_unique<OpenInFolderCommand>();
}

} // namespace app
