// Besprited | Copyright (C) 2026 Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

// A template's inline member bodies aren't checked until the template is
// instantiated, but instantiation happens wherever a derived class's own
// definition needs DragState<Base>'s complete vtable - which, for
// MovingSymmetryState/MovingCelState, is inside their own header, before
// their .cpp gets around to including editor.h. Include it here instead of
// relying on every includer's own include order.
#include "app/ui/editor/editor.h"
#include "gfx/point.h"
#include "ui/message.h"

namespace app
{

// Shared shell for editor states that drag something (a cel, a symmetry
// axis line, ...) around while the mouse button is held down: capture the
// mouse and a start position on entry, recompute a delta (in editor space)
// against that start position on every onMouseMove() and hand it to
// onDrag(), and on mouse-up call onDragEnd() then go back to the previous
// state and release the mouse. MovingSymmetryState and MovingCelState both
// hand-wrote this exact shell. See issue #225.
//
// ScrollingState also builds on this, but needs screen-space positions
// (override dragPos()) and a Windows-only autoScroll() short-circuit
// (override skipDrag()). It also re-bases the drag start on every move
// (rebaseDrag()) so its deltas stay incremental, as the view's scroll
// position gets clamped. See issue #252.
//
// Base is whichever EditorState subclass this drag state should otherwise
// behave like - StandbyState for a state that falls through to
// StandbyState::onMouseMove() for status-bar/brush-preview updates once the
// drag itself is applied (see afterDrag()).
template <class Base> class DragState : public Base
{
public:
  bool onMouseUp(Editor* editor, ui::MouseMessage* msg) override
  {
    onDragEnd(editor);
    editor->backToPreviousState();
    editor->releaseMouse();
    return true;
  }

  bool onMouseMove(Editor* editor, ui::MouseMessage* msg) override
  {
    if (skipDrag(editor, msg))
      return true;
    onDrag(editor, dragPos(editor, msg) - m_start);
    return afterDrag(editor, msg);
  }

protected:
  // Captures the mouse and records the drag's start point, in editor space.
  void beginDrag(Editor* editor, const gfx::Point& startEditorPos)
  {
    m_start = startEditorPos;
    editor->captureMouse();
  }

  const gfx::Point& dragStart() const { return m_start; }

  // Moves the drag's start point, so later deltas are relative to it.
  void rebaseDrag(const gfx::Point& newStart) { m_start = newStart; }

  // The mouse position the drag delta is computed from. Editor space by
  // default; override to use another space (e.g. screen space), and pass
  // a start point in that same space to beginDrag().
  virtual gfx::Point dragPos(Editor* editor, ui::MouseMessage* msg) const
  {
    return editor->screenToEditor(msg->position());
  }

  // Called first on every onMouseMove(); return true to swallow the move
  // (no onDrag()/afterDrag()). The state may rebaseDrag() here.
  virtual bool skipDrag(Editor* editor, ui::MouseMessage* msg)
  {
    return false;
  }

  // Applies whatever this state drags, given the delta (in editor space)
  // between the current mouse position and the drag's start point.
  // Implementations are expected to editor->invalidate() if they change
  // something the editor needs to redraw.
  virtual void onDrag(Editor* editor, const gfx::Point& delta) = 0;

  // Called once from onMouseUp(), before backToPreviousState()/
  // releaseMouse() - e.g. MovingCelState commits its transaction here.
  virtual void onDragEnd(Editor* editor) {}

  // Called after onDrag() on every onMouseMove() - lets a state chain into
  // its "normal" (non-dragging) onMouseMove behavior, e.g.
  // StandbyState::onMouseMove()'s status-bar/brush-preview updates.
  virtual bool afterDrag(Editor* editor, ui::MouseMessage* msg)
  {
    return true;
  }

private:
  gfx::Point m_start;
};

} // namespace app
