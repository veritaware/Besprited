// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/commands/cmd_set_palette.h"
#include "app/context_access.h"
#include "app/document_api.h"
#include "app/file_selector.h"
#include "app/ini_file.h"
#include "app/modules/palettes.h"
#include "app/transaction.h"
#include "doc/color.h"
#include "doc/palette.h"
#include "doc/sprite.h"
#include "ui/alert.h"
#include "ui/manager.h"

#include <memory>

namespace app
{

using namespace ui;

SetPaletteCommand::SetPaletteCommand()
  : Command("SetPalette", "Set Palette", CmdRecordableFlag)
  , m_palette(nullptr)
{
}

void SetPaletteCommand::onExecute(Context* context)
{
  ASSERT(m_palette);
  if (!m_palette)
    return;

  const bool reset = m_resetTransparentIndex;
  m_resetTransparentIndex = false;

  std::shared_ptr<doc::Palette> normalized;
  const doc::Palette* palette = m_palette;

  ContextWriter writer(context);
  if (writer.document())
  {
    Transaction transaction(writer.context(), "Set Palette");
    DocumentApi api = writer.document()->getApi(transaction);
    const bool indexed = (writer.sprite()->pixelFormat() == IMAGE_INDEXED);
    if (reset && indexed)
    {
      normalized = m_palette->clone();
      normalized->ensureTransparentFirst();
      palette = normalized.get();
    }
    api.setPalette(writer.sprite(), writer.frame(), palette);
    if (normalized && writer.sprite()->transparentColor() != 0)
      api.setSpriteTransparentColor(writer.sprite(), 0);
    transaction.commit();
  }
  set_current_palette(palette, false);

  // Redraw the entire screen
  ui::Manager::getDefault()->invalidate();
}

std::unique_ptr<Command> CommandFactory::createSetPaletteCommand()
{
  return std::make_unique<SetPaletteCommand>();
}

} // namespace app
