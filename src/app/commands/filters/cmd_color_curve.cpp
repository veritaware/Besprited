// Aseprite  | Copyright (C) 2001-2016 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/color.h"
#include "app/commands/command.h"
#include "app/commands/filters/color_curve_editor.h"
#include "app/commands/filters/filter_manager_impl.h"
#include "app/commands/filters/filter_window.h"
#include "app/context.h"
#include "app/ini_file.h"
#include "app/modules/gui.h"
#include "app/ui/color_button.h"
#include "base/bind.h"
#include "doc/mask.h"
#include "doc/sprite.h"
#include "filters/color_curve.h"
#include "filters/color_curve_filter.h"
#include "ui/ui.h"

namespace app
{

using namespace filters;

class ColorCurveWindow : public FilterWindow
{
public:
  ColorCurveWindow(ColorCurveFilter& filter, FilterManagerImpl& filterMgr)
    : FilterWindow("Color Curve", "ColorCurve", &filterMgr,
                   WithChannelsSelector, WithoutTiledCheckBox)
    , m_filter(filter)
    , m_editor(filter.getCurve(), gfx::Rect(0, 0, 256, 256))
    , m_resetButton("&Reset")
  {
    m_view.attachToView(&m_editor);
    m_view.setExpansive(true);
    m_view.setMinSize(gfx::Size(128, 64));

    getContainer()->addChild(&m_view);

    insertExtraButton(&m_resetButton);

    m_editor.CurveEditorChange.connect(&ColorCurveWindow::onCurveChange, this);
    m_resetButton.Click.connect(&ColorCurveWindow::onReset, this);
  }

protected:
  void onCurveChange()
  {
    // The color curve in the filter is the same refereced by the
    // editor. But anyway, we have to re-set the same curve in the
    // filter to regenerate the map used internally by the filter
    // (which is calculated inside setCurve() method).
    m_filter.setCurve(m_editor.getCurve());

    restartPreview();
  }

  void onReset(ui::Event& ev)
  {
    m_editor.resetToDefault();
  }

private:
  ColorCurveFilter& m_filter;
  ui::View m_view;
  ColorCurveEditor m_editor;
  ui::Button m_resetButton;
};

class ColorCurveCommand : public Command
{
public:
  ColorCurveCommand();

protected:
  bool onEnabled(Context* context) override;
  void onExecute(Context* context) override;
};

ColorCurveCommand::ColorCurveCommand()
  : Command("ColorCurve", "Color Curve", CmdRecordableFlag)
{
}

bool ColorCurveCommand::onEnabled(Context* context)
{
  return context->checkFlags(ContextFlags::ActiveDocumentIsWritable |
                             ContextFlags::HasActiveSprite);
}

void ColorCurveCommand::onExecute(Context* context)
{
  // The curve always starts as the default linear identity curve
  // (no user-set points) each time the dialog is opened, regardless
  // of whether the previous invocation was accepted or cancelled.
  ColorCurve curve = ColorCurve::Default();

  ColorCurveFilter filter;
  filter.setCurve(&curve);

  FilterManagerImpl filterMgr(context, &filter);
  filterMgr.setTarget(TARGET_RED_CHANNEL | TARGET_GREEN_CHANNEL |
                      TARGET_BLUE_CHANNEL | TARGET_GRAY_CHANNEL |
                      TARGET_ALPHA_CHANNEL);

  ColorCurveWindow window(filter, filterMgr);
  window.doModal();
}

std::unique_ptr<Command> CommandFactory::createColorCurveCommand()
{
  return std::make_unique<ColorCurveCommand>();
}

} // namespace app
