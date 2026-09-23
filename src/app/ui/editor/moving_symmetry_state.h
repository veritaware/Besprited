// Aseprite    | Copyright (C) 2015-2016 David Capello
// LibreSprite | Copyright (C) 2021      LibreSprite contributors
// Besprited   | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "app/pref/preferences.h"
#include "app/ui/editor/drag_state.h"
#include "app/ui/editor/standby_state.h"
#include "app/ui/editor/symmetry_handles.h"

namespace app
{
class Editor;

class MovingSymmetryState : public DragState<StandbyState>
{
public:
  MovingSymmetryState(Editor* editor, ui::MouseMessage* msg, Axis axis,
                      Option<int>& xAxis, Option<int>& yAxis);
  virtual ~MovingSymmetryState();

  virtual bool onUpdateStatusBar(Editor* editor) override;

  virtual bool requireBrushPreview() override { return false; }

protected:
  void onDrag(Editor* editor, const gfx::Point& delta) override;
  bool afterDrag(Editor* editor, ui::MouseMessage* msg) override
  {
    return StandbyState::onMouseMove(editor, msg);
  }

private:
  Axis m_symmetryAxis;
  Option<int>& m_xAxis;
  Option<int>& m_yAxis;
  int m_xAxisStart;
  int m_yAxisStart;
};

} // namespace app
