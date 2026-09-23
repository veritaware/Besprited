// LibreSprite | Copyright (C) 2023 LibreSprite contributors
// Besprited   | Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/app.h"
#include "app/commands/command.h"
#include "app/commands/params.h"
#include "app/modules/editors.h"
#include "app/pref/preferences.h"
#include "app/ui/editor/editor.h"
#include "base/convert_to.h"
#include "ui/manager.h"
#include "ui/system.h"
#include "she/display.h"
#include "app/ui/main_window.h"

namespace app
{

class AlternateTouchbarCommand : public Command
{
public:

  AlternateTouchbarCommand()
    : Command{"AlternateTouchbar", "Alternate Touchbar", CmdUIOnlyFlag}
  {
  }

protected:
  void onLoadParams(const Params& params) override {}

  bool onEnabled(Context* context) override { return true; }

  bool onChecked(Context* context) override
  {
    return Preferences::instance().touchBar.alternatePosition();
  }

  void onExecute(Context* context) override
  {
    App::instance()->mainWindow()->alternateTouchbar();
  }
};

std::unique_ptr<Command> CommandFactory::createAlternateTouchbarCommand()
{
  return std::make_unique<AlternateTouchbarCommand>();
}

} // namespace app
