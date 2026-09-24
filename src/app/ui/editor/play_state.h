// Aseprite  | Copyright (C) 2001-2016 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "app/pref/option.h"
#include "app/ui/editor/state_with_wheel_behavior.h"
#include "base/connection.h"
#include "base/time.h"
#include "doc/frame.h"
#include "ui/timer.h"

namespace app
{

class CommandExecutionEvent;

// Shows a popup menu to change editor's animation playback speed (and, for
// the main editor, play-once/rewind-on-stop behavior). Used by both
// Editor::showAnimationSpeedMultiplierPopup() and PreviewEditor's mini
// toolbar (app/ui/ani_controls.cpp) - moved here from Editor since it's
// playback UI, not editor-state machinery, and PlayState already owns the
// rest of the playback logic (see issue #225).
void show_animation_speed_multiplier_popup(Editor* editor,
                                           Option<bool>& playOnce,
                                           bool withStopBehaviorOptions);

class PlayState : public StateWithWheelBehavior
{
public:
  PlayState(bool playOnce);

  void onEnterState(Editor* editor) override;
  LeaveAction onLeaveState(Editor* editor, EditorState* newState) override;
  bool onMouseDown(Editor* editor, ui::MouseMessage* msg) override;
  bool onMouseUp(Editor* editor, ui::MouseMessage* msg) override;
  bool onMouseMove(Editor* editor, ui::MouseMessage* msg) override;
  bool onKeyDown(Editor* editor, ui::KeyMessage* msg) override;
  bool onKeyUp(Editor* editor, ui::KeyMessage* msg) override;

private:
  void onPlaybackTick();

  // ContextObserver
  void onBeforeCommandExecution(CommandExecutionEvent& ev);

  double getNextFrameTime();

  Editor* m_editor;
  bool m_playOnce;
  bool m_toScroll;
  ui::Timer m_playTimer;

  // Number of milliseconds to go to the next frame if m_playTimer
  // is activated.
  double m_nextFrameTime;
  base::tick_t m_curFrameTick;

  bool m_pingPongForward;
  doc::frame_t m_refFrame;

  base::ScopedConnection m_ctxConn;
};

} // namespace app
