// Undo Library
// Copyright (C) 2015-2016 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

namespace undo {

  class UndoCommand {
  public:
    virtual ~UndoCommand() { }
    virtual void undo() = 0;
    virtual void redo() = 0;
    virtual void dispose() = 0;
  };

} // namespace undo
