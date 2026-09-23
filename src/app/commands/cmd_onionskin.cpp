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
#include "app/context.h"
#include "app/document.h"
#include "app/pref/preferences.h"

namespace app
{

using namespace gfx;

class ShowOnionSkinCommand : public Command
{
public:
  ShowOnionSkinCommand()
    : Command("ShowOnionSkin", "Show Onion Skin", CmdUIOnlyFlag)
  {
  }


protected:
  bool onChecked(Context* context) override
  {
    DocumentPreferences& docPref =
        Preferences::instance().document(context->activeDocument());
    return docPref.onionskin.active();
  }

  void onExecute(Context* context) override
  {
    DocumentPreferences& docPref =
        Preferences::instance().document(context->activeDocument());
    docPref.onionskin.active(!docPref.onionskin.active());
  }
};

std::unique_ptr<Command> CommandFactory::createShowOnionSkinCommand()
{
  return std::make_unique<ShowOnionSkinCommand>();
}

} // namespace app
