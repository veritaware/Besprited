// Besprited | Copyright (C) 2026 Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "app/app.h"
#include "app/pref/preferences.h"
#include "app/tools/tool.h"
#include "app/ui/context_bar.h"

namespace app
{

// Mixin for a ContextBar tool-option widget (IntEntry/CheckBox-derived)
// whose entire write-back body is "push this widget's value into a single
// Option<T> member of the active tool's ToolPreferences, unless
// ContextBar::updateForTool() is currently loading that same value into the
// widget" - the ~identical body that ToleranceField, ContiguousField and
// several other context_bar.cpp fields used to hand-write. See issue #227.
//
// WidgetBase's constructors are inherited as-is; a derived field just adds
// the WidgetBase event handler override and calls commit() from it, e.g.:
//
//   class ContextBar::ToleranceField
//     : public ToolPrefField<IntEntry, int, &ToolPreferences::tolerance>
//   {
//   public:
//     ToleranceField() : ToolPrefField(0, 255) {}
//   protected:
//     void onValueChange() override
//     {
//       IntEntry::onValueChange();
//       commit(getValue());
//     }
//   };
template <class WidgetBase, class T, app::Option<T> ToolPreferences::*Member>
class ToolPrefField : public WidgetBase
{
protected:
  using WidgetBase::WidgetBase;

  void commit(const T& value)
  {
    if (ContextBar::updatingFromCode())
      return;

    tools::Tool* tool = App::instance()->activeTool();
    (Preferences::instance().tool(tool).*Member)(value);
  }
};

} // namespace app
