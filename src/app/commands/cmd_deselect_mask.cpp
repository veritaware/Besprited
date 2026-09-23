// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/cmd/deselect_mask.h"
#include "app/commands/command.h"
#include "app/context_access.h"
#include "app/modules/gui.h"
#include "app/transaction.h"
#include "doc/mask.h"
#include "doc/sprite.h"

namespace app
{

class DeselectMaskCommand : public Command
{
public:
  DeselectMaskCommand();

protected:
  bool onEnabled(Context* context) override;
  void onExecute(Context* context) override;
};

DeselectMaskCommand::DeselectMaskCommand()
  : Command("DeselectMask", "Deselect Mask", CmdRecordableFlag)
{
}

bool DeselectMaskCommand::onEnabled(Context* context)
{
  return context->checkFlags(ContextFlags::ActiveDocumentIsWritable |
                             ContextFlags::HasVisibleMask);
}

void DeselectMaskCommand::onExecute(Context* context)
{
  ContextWriter writer(context);
  Document* document(writer.document());
  {
    Transaction transaction(writer.context(), "Deselect", DoesntModifyDocument);
    transaction.execute(new cmd::DeselectMask(document));
    transaction.commit();
  }
  document->generateMaskBoundaries();
  update_screen_for_document(document);
}

std::unique_ptr<Command> CommandFactory::createDeselectMaskCommand()
{
  return std::make_unique<DeselectMaskCommand>();
}

} // namespace app
