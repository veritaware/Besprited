// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "app/ui/editor/drag_state.h"
#include "app/ui/editor/standby_state.h"

#include "app/context_access.h"
#include "doc/cel_list.h"

#include <vector>

namespace doc
{
class Cel;
}

namespace app
{
class Editor;

class MovingCelState : public DragState<StandbyState>
{
public:
  MovingCelState(Editor* editor, ui::MouseMessage* msg);
  virtual ~MovingCelState();

  virtual bool onUpdateStatusBar(Editor* editor) override;

  virtual bool requireBrushPreview() override { return false; }

protected:
  void onDrag(Editor* editor, const gfx::Point& delta) override;
  void onDragEnd(Editor* editor) override;
  bool afterDrag(Editor* editor, ui::MouseMessage* msg) override
  {
    return StandbyState::onMouseMove(editor, msg);
  }

private:
  ContextReader m_reader;
  CelList m_celList;
  std::vector<gfx::Point> m_celStarts;
  gfx::Point m_celOffset;
  bool m_canceled;
  bool m_maskVisible;
};

} // namespace app
