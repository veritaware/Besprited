// Aseprite  | Copyright (C) 2001-2016 David Capello
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
#include "app/modules/gui.h"
#include "app/ui/timeline.h"
#include "app/document_range_ops.h"

namespace app
{

class ReverseFramesCommand : public Command
{
public:
  ReverseFramesCommand();

protected:
  bool onEnabled(Context* context) override;
  void onExecute(Context* context) override;
};

ReverseFramesCommand::ReverseFramesCommand()
  : Command("ReverseFrames", "Reverse Frames", CmdUIOnlyFlag)
{
}

bool ReverseFramesCommand::onEnabled(Context* context)
{
  auto range = App::instance()->timeline()->range();
  return context->checkFlags(ContextFlags::ActiveDocumentIsWritable) &&
         range.enabled() &&
         range.frames() >= 2; // We need at least 2 frames to reverse
}

void ReverseFramesCommand::onExecute(Context* context)
{
  auto range = App::instance()->timeline()->range();
  if (!range.enabled())
    return; // Nothing to do

  Document* doc = context->activeDocument();

  reverse_frames(doc, range);

  update_screen_for_document(doc);
}

std::unique_ptr<Command> CommandFactory::createReverseFramesCommand()
{
  return std::make_unique<ReverseFramesCommand>();
}

} // namespace app
