// UI Library
// Besprited | Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ui/number_entry.h"

#include "base/base.h"
#include "evalmath/evalmath.h"
#include "ui/message.h"

#include <algorithm>
#include <climits>
#include <cmath>
#include <cstdio>

namespace ui {

NumberEntry::NumberEntry(int min, int max)
  : Entry(32, "")
  , m_min(min)
  , m_max(max)
  , m_lastValidValue(min)
{
}

void NumberEntry::setValue(int value)
{
  value = MID(m_min, value, m_max);
  m_lastValidValue = value;

  char buf[32];
  snprintf(buf, sizeof(buf), "%d", value);
  setText(buf);
}

bool NumberEntry::onProcessMessage(Message* msg)
{
  if (msg->type() == kFocusLeaveMessage) {
    auto val = evalmath::eval(text());
    if (val) {
      long rounded = std::lround(val.value());
      setValue(static_cast<int>(std::clamp<long>(rounded, INT_MIN, INT_MAX)));
    }
    else {
      // Unparseable input: revert to the last known-good value
      // instead of silently defaulting to some arbitrary number.
      setValue(m_lastValidValue);
    }
    deselectText();
  }

  return Entry::onProcessMessage(msg);
}

} // namespace ui
