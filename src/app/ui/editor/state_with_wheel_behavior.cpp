// Aseprite  | Copyright (C) 2001-2016 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/ui/editor/state_with_wheel_behavior.h"

#include "app/app.h"
#include "app/commands/commands.h"
#include "app/commands/params.h"
#include "app/modules/palettes.h"
#include "app/pref/preferences.h"
#include "app/ui/color_bar.h"
#include "app/ui/editor/editor.h"
#include "app/ui/keyboard_shortcuts.h"
#include "app/ui_context.h"
#include "doc/palette.h"
#include "ui/message.h"
#include "ui/theme.h"

#include <algorithm>

namespace app
{

using namespace ui;

bool StateWithWheelBehavior::onMouseWheel(Editor* editor, MouseMessage* msg)
{
  using Action = KeyAction;

  gfx::Point delta = msg->wheelDelta();
  // With a modifier the wheel acts on a single axis, so horizontal and
  // vertical scrolling behave the same.
  double dz = delta.x + delta.y;
  const bool noModifiers = (msg->modifiers() & (kKeyShiftModifier | kKeyCtrlModifier | kKeyAltModifier |
                                                kKeyCmdModifier | kKeyWinModifier)) == 0;
  const Action wheelAction = KeyboardShortcuts::instance()->getWheelAction(msg->modifiers());

  auto stepColor = [dz](const app::Color& color, auto setColor)
  {
    int lastIndex = get_current_palette()->size() - 1;
    int newIndex = MID(0, color.getIndex() + int(dz), lastIndex);
    setColor(app::Color::fromIndex(newIndex));
  };

  switch (wheelAction)
  {

  case Action::WheelFgColor:
    stepColor(ColorBar::instance()->getFgColor(), [](const app::Color& c) { ColorBar::instance()->setFgColor(c); });
    break;

  case Action::WheelBgColor:
    stepColor(ColorBar::instance()->getBgColor(), [](const app::Color& c) { ColorBar::instance()->setBgColor(c); });
    break;

  case Action::WheelBrushSize:
  {
    // Wheel up has a negative delta: by default it grows the brush.
    if (!Preferences::instance().editor.invertWheelBrushSize())
      dz = -dz;
    if (int(dz) == 0)
      break;
    Command* command = CommandsModule::instance()->getCommandByName(CommandId::ChangeBrush);
    if (command)
    {
      Params params;
      params.set("change", dz > 0 ? "increment-size" : "decrement-size");
      UIContext::instance()->executeCommand(command, params);
    }
    break;
  }

  case Action::WheelFrame:
  {
    Command* command = CommandsModule::instance()->getCommandByName((dz < 0.0) ? CommandId::GotoNextFrame
                                                                               : CommandId::GotoPreviousFrame);
    if (command)
      UIContext::instance()->executeCommand(command);
    break;
  }

  case Action::WheelZoom:
  {
    render::Zoom zoom = editor->zoom();

    if (msg->preciseWheel())
      dz = std::clamp(dz / 1.5, -1.0, 1.0);

    zoom = render::Zoom::fromLinearScale(zoom.linearScale() - int(dz));

    setZoom(editor, zoom, msg->position());
    break;
  }

  case Action::WheelScrollH:
  case Action::WheelScrollV:
  {
    View* view = View::getView(editor);
    gfx::Point scroll = view->viewScroll();

    // Without modifiers both wheel axes pan their own direction.
    if (wheelAction == Action::WheelScrollH)
      delta = gfx::Point(int(dz), 0);
    else if (!noModifiers)
      delta = gfx::Point(0, int(dz));

    if (!msg->preciseWheel())
    {
      gfx::Rect vp = view->viewportBounds();
      delta.x = delta.x * vp.w / 10;
      delta.y = delta.y * vp.h / 10;
    }

    if (Preferences::instance().editor.invertHorizontalScroll())
      delta.x = -delta.x;
    if (Preferences::instance().editor.invertVerticalScroll())
      delta.y = -delta.y;

    editor->setEditorScroll(scroll + delta);
    break;
  }

  default:
    break;
  }

  return true;
}

bool StateWithWheelBehavior::onTouchMagnify(Editor* editor, ui::TouchMessage* msg)
{
  render::Zoom zoom = editor->zoom();
  // The internal scale accumulates small pinches while the displayed zoom
  // snaps to the zoom table.
  zoom = render::Zoom::fromScale(zoom.internalScale() * std::max(0.01, 1.0 + msg->magnification()));

  setZoom(editor, zoom, msg->position());
  return true;
}

void StateWithWheelBehavior::setZoom(Editor* editor, const render::Zoom& zoom, const gfx::Point& mousePos)
{
  bool center = Preferences::instance().editor.zoomFromCenterWithWheel();

  editor->setZoomAndCenterInMouse(zoom, mousePos,
                                  (center ? Editor::ZoomBehavior::CENTER : Editor::ZoomBehavior::MOUSE));
}

} // namespace app
