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
#include "app/cmd/set_palette.h"
#include "app/commands/command.h"
#include "app/commands/params.h"
#include "app/console.h"
#include "app/context.h"
#include "app/context_access.h"
#include "app/job.h"
#include "app/modules/palettes.h"
#include "app/pref/preferences.h"
#include "app/transaction.h"
#include "app/ui/color_bar.h"
#include "app/ui_context.h"
#include "base/log.h"
#include "doc/palette.h"
#include "doc/sprite.h"
#include "render/quantization.h"
#include "ui/manager.h"

#include "palette_from_sprite.xml.h"

#include <memory>

namespace app
{

class ColorQuantizationCommand : public Command
{
public:
  ColorQuantizationCommand();

protected:
  void onLoadParams(const Params& params) override;
  std::vector<CommandParamInfo> onDescribeParams() const override;
  bool onEnabled(Context* context) override;
  void onExecute(Context* context) override;

private:
  int m_colors;
  bool m_alpha;
  bool m_hasAlpha; // true when the alpha param is given

  void executeWithoutDialog(Context* context);

  void reset()
  {
    m_colors = 0;
    m_alpha = false;
    m_hasAlpha = false;
  }
};

class ColorQuantizationJob : public Job,
                             public render::PaletteOptimizerDelegate
{
public:
  ColorQuantizationJob(Sprite* sprite, bool withAlpha, Palette& palette)
    : Job("Creating Palette")
    , m_sprite(sprite)
    , m_withAlpha(withAlpha)
    , m_palette(palette)
  {
  }

private:
  void onJob() override
  {
    render::create_palette_from_sprite(m_sprite, 0, m_sprite->lastFrame(), m_withAlpha, &m_palette, this);
  }
  bool onPaletteOptimizerContinue() override { return !isCanceled(); }
  void onPaletteOptimizerProgress(double progress) override { jobProgress(progress); }

  Sprite* m_sprite;
  bool m_withAlpha;
  Palette& m_palette;
};

namespace
{

// Runs the quantization job; returns nullptr if canceled.
std::shared_ptr<Palette> quantize_sprite(Sprite* sprite, frame_t frame, bool withAlpha, int ncolors)
{
  auto palette = Palette::create(ncolors);
  palette->setFrame(frame);
  ColorQuantizationJob job(sprite, withAlpha, *palette);
  job.startJob();
  job.waitJob();
  return job.isCanceled() ? nullptr : palette;
}

// Applies the palette to the sprite as an undoable change.
void commit_palette(Sprite* sprite, frame_t frame, const Palette* curPalette, Palette* newPalette)
{
  if (*curPalette == *newPalette)
    return;

  ContextWriter writer(UIContext::instance(), 500);
  Transaction transaction(writer.context(), "Color Quantization", ModifyDocument);
  transaction.execute(new cmd::SetPalette(sprite, frame, *newPalette));
  transaction.commit();

  set_current_palette(newPalette, false);
  ui::Manager::getDefault()->invalidate();
}

} // namespace

ColorQuantizationCommand::ColorQuantizationCommand()
  : Command("ColorQuantization", "Create Palette from Current Sprite (Color Quantization)", CmdRecordableFlag)
{
  reset();
}

void ColorQuantizationCommand::onLoadParams(const Params& params)
{
  reset();
  if (params.has_param("colors"))
    m_colors = params.get_as<int>("colors");
  if (params.has_param("alpha"))
  {
    m_alpha = (params.get("alpha") == "true");
    m_hasAlpha = true;
  }
}

std::vector<CommandParamInfo> ColorQuantizationCommand::onDescribeParams() const
{
  return {
      {"colors", "Size of the new quantized palette. With this, runs without the dialog.", false, ""},
      {"alpha", "Keep the alpha channel in the palette (true or false).", false, ""},
  };
}

bool ColorQuantizationCommand::onEnabled(Context* context)
{
  return context->checkFlags(ContextFlags::ActiveDocumentIsWritable);
}

void ColorQuantizationCommand::executeWithoutDialog(Context* context)
{
  Sprite* sprite;
  frame_t frame;
  Palette* curPalette;
  {
    ContextReader reader(context);
    Site site = context->activeSite();
    sprite = site.sprite();
    frame = site.frame();
    curPalette = sprite->palette(frame);
  }

  bool withAlpha = m_hasAlpha ? m_alpha : App::instance()->preferences().quantization.withAlpha();

  auto tmpPalette = quantize_sprite(sprite, frame, withAlpha, m_colors);
  if (tmpPalette)
    commit_palette(sprite, frame, curPalette, tmpPalette.get());
}

void ColorQuantizationCommand::onExecute(Context* context)
{
  try
  {
    // With the colors param, run the "new palette" path without the dialog.
    if (m_colors > 0)
    {
      executeWithoutDialog(context);
      return;
    }

    // The dialog would block headless.
    if (failIfHeadless())
      return;

    app::gen::PaletteFromSprite window;
    PalettePicks entries;

    Sprite* sprite;
    frame_t frame;
    Palette* curPalette;
    {
      ContextReader reader(context);
      Site site = context->activeSite();
      sprite = site.sprite();
      frame = site.frame();
      curPalette = sprite->palette(frame);

      window.newPalette()->setSelected(true);
      window.alphaChannel()->setSelected(App::instance()->preferences().quantization.withAlpha());
      window.ncolors()->setValue(256);

      ColorBar::instance()->getPaletteView()->getSelectedEntries(entries);
      if (entries.picks() > 1)
      {
        window.currentRange()->setTextf("%s, %d color(s)", window.currentRange()->text().c_str(), entries.picks());
      }
      else
        window.currentRange()->setEnabled(false);

      window.currentPalette()->setTextf("%s, %d color(s)", window.currentPalette()->text().c_str(), curPalette->size());
    }

    window.openWindowInForeground();
    if (window.closer() != window.ok())
      return;

    bool withAlpha = window.alphaChannel()->isSelected();
    App::instance()->preferences().quantization.withAlpha(withAlpha);

    bool createPal = false;
    if (window.newPalette()->isSelected())
    {
      int n = window.ncolors()->getValue();
      entries = PalettePicks(n);
      entries.all();
      createPal = true;
    }
    else if (window.currentPalette()->isSelected())
    {
      entries.all();
    }
    if (entries.picks() == 0)
      return;

    auto tmpPalette = quantize_sprite(sprite, frame, withAlpha, entries.picks());
    if (!tmpPalette)
      return;

    auto newPalette = createPal ? tmpPalette.get() : get_current_palette();

    if (createPal)
    {
      entries = PalettePicks(newPalette->size());
      entries.all();
    }

    int i = 0, j = 0;
    for (bool state : entries)
    {
      if (state)
        newPalette->setEntry(i, tmpPalette->getEntry(j++));
      ++i;
    }

    commit_palette(sprite, frame, curPalette, newPalette);
  }
  catch (base::Exception& e)
  {
    Console::showException(e);
  }
}

std::unique_ptr<Command> CommandFactory::createColorQuantizationCommand()
{
  return std::make_unique<ColorQuantizationCommand>();
}

} // namespace app
