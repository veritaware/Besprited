// Besprited | Copyright (C) 2026 Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/app.h"
#include "app/commands/command.h"
#include "app/modules/editors.h"
#include "app/ui/editor/editor.h"
#include "doc/sprite.h"
#include "render/zoom.h"
#include "ui/view.h"

#include <algorithm>

namespace app {

class FitScreenCommand : public Command {
public:
  FitScreenCommand();
  Command* clone() const override { return new FitScreenCommand(*this); }

protected:
  bool onEnabled(Context* context) override;
  void onExecute(Context* context) override;
};

FitScreenCommand::FitScreenCommand()
  : Command("FitScreen",
            "Fit Screen",
            CmdUIOnlyFlag)
{
}

bool FitScreenCommand::onEnabled(Context* context)
{
  return (current_editor != NULL && current_editor->sprite() != NULL);
}

void FitScreenCommand::onExecute(Context* context)
{
  Editor* editor = current_editor;
  doc::Sprite* sprite = editor->sprite();
  if (!sprite)
    return;

  ui::View* view = ui::View::getView(editor);
  gfx::Rect vp = view->viewportBounds();

  double scaleX = vp.w / (double)sprite->width();
  double scaleY = vp.h / (double)sprite->height();
  double scale = std::min(scaleX, scaleY);

  render::Zoom zoom = render::Zoom::fromScale(scale);

  editor->setZoomAndCenterInMouse(
    zoom,
    gfx::Point(vp.x + vp.w/2, vp.y + vp.h/2),
    Editor::ZoomBehavior::CENTER);
}

Command* CommandFactory::createFitScreenCommand()
{
  return new FitScreenCommand;
}

} // namespace app
