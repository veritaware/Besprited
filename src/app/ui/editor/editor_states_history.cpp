// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/ui/editor/editor_states_history.h"

namespace app
{

EditorStatesHistory::EditorStatesHistory() = default;

EditorStatesHistory::~EditorStatesHistory()
{
  clear();
}

EditorStatePtr EditorStatesHistory::top()
{
  return !m_states.empty() ? m_states.back() : nullptr;
}

void EditorStatesHistory::push(const EditorStatePtr& state)
{
  ASSERT(state);
  m_states.push_back(state);
}

void EditorStatesHistory::pop()
{
  ASSERT(!m_states.empty());
  // cppcheck-suppress containerOutOfBounds
  m_states.pop_back();
}

void EditorStatesHistory::clear()
{
  // EditorState (base or any subclass in src/app/ui/editor/) never holds a
  // reference to another EditorState, so nothing depends on the order these
  // are destroyed in - just let the vector destroy its elements normally.
  m_states.clear();
}

} // namespace app
