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
#include "app/commands/commands.h"
#include "app/commands/param_names.h"
#include "app/commands/params.h"
#include "app/context.h"
#include "app/file/palette_file.h"
#include "app/file_selector.h"
#include "app/modules/palettes.h"
#include "base/fs.h"
#include "base/path.h"
#include "base/string.h"
#include "doc/color.h"
#include "doc/palette.h"
#include "ui/alert.h"

#include <cstdint>

namespace app
{

using namespace ui;

class SavePaletteCommand : public Command
{
public:
  SavePaletteCommand();

protected:
  void onLoadParams(const Params& params) override;
  std::vector<CommandParamInfo> onDescribeParams() const override;
  void onExecute(Context* context) override;

private:
  std::string m_preset;
};

SavePaletteCommand::SavePaletteCommand()
  : Command("SavePalette", "Save Palette", CmdRecordableFlag)
{
}

void SavePaletteCommand::onLoadParams(const Params& params)
{
  m_preset = params.get(param::kPreset);
}

std::vector<CommandParamInfo> SavePaletteCommand::onDescribeParams() const
{
  return {
      {param::kPreset, "Preset palette name to save to. Empty opens the file dialog.", false, ""},
  };
}

void SavePaletteCommand::onExecute(Context* context)
{
  const doc::Palette* palette = get_current_palette();
  std::string filename;

  if (!m_preset.empty())
  {
    filename = get_preset_palette_filename(m_preset, ".ase");
  }
  else
  {
    std::string exts = get_writable_palette_extensions();
    filename = app::show_file_selector("Save Palette", "", exts, FileSelectorType::Save);
    if (filename.empty())
      return;
  }

  const std::string ext = base::string_to_lower(base::get_file_extension(filename));
  if (ext == "act")
  {
    if (palette->size() > 256)
    {
      Alert::show("Error<<Adobe Color Table palettes support at most 256 colors.||&Close");
      return;
    }

    int transparentEntries = 0;
    bool partiallyTransparent = false;
    for (int i = 0; i < palette->size(); ++i)
    {
      const uint8_t alpha = rgba_geta(palette->entry(i));
      partiallyTransparent |= alpha != 0 && alpha != 255;
      transparentEntries += alpha == 0;
    }
    if (partiallyTransparent)
    {
      Alert::show("Error<<Adobe Color Table palettes do not support partial transparency.||&Close");
      return;
    }
    if (transparentEntries > 1)
    {
      Alert::show("Error<<Adobe Color Table palettes support only one transparent color.||&Close");
      return;
    }
  }

  if (ext == "txt" && palette->size() > 96 &&
      Alert::show("Warning"
                  "<<Paint.NET palettes are limited to 96 colors. This palette has %d; some entries may be ignored."
                  "||&Save Anyway||&Cancel",
                  palette->size()) != 1)
    return;

  if (!save_palette(filename.c_str(), *palette,
                    16)) // TODO 16 should be configurable
    Alert::show("Error<<Saving palette file||&Close");

  if (m_preset == get_default_palette_preset_name())
  {
    set_default_palette(palette);
    if (!context->activeDocument())
      set_current_palette(palette, false);
  }
}

std::unique_ptr<Command> CommandFactory::createSavePaletteCommand()
{
  return std::make_unique<SavePaletteCommand>();
}

} // namespace app
