// UI Library
// Besprited | Copyright (C) 2026       Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include "ui/entry.h"

namespace ui {

  // A text entry for integer values that accepts evalmath expressions
  // (e.g. "16*2") and validates the text when focus is lost: a
  // parseable value is clamped into [min, max], an unparseable one
  // reverts the field to the last known-good value instead of
  // silently defaulting to some arbitrary number.
  class NumberEntry : public Entry {
  public:
    NumberEntry(int min, int max);

    int getValue() const { return m_lastValidValue; }
    void setValue(int value);
    int min() const { return m_min; }
    int max() const { return m_max; }
    void setMin(int value) { m_min = value; }
    void setMax(int value) { m_max = value; }

  protected:
    bool onProcessMessage(Message* msg) override;

  private:
    int m_min;
    int m_max;
    int m_lastValidValue;
  };

} // namespace ui
