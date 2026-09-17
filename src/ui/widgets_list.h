// UI Library
// Aseprite  | Copyright (C) 2001-2013, 2015 David Capello
// Besprited | Copyright (C) 2026            Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <vector>

#define UI_FOREACH_WIDGET_BACKWARD(list_name, iterator_name)                   \
  for (WidgetsList::const_reverse_iterator                                     \
           iterator_name = (list_name).rbegin(),                               \
           __end = (list_name).rend();                                         \
       iterator_name != __end; ++iterator_name)

#define UI_FOREACH_WIDGET_WITH_END(list_name, iterator_name, end_name)         \
  for (WidgetsList::const_iterator iterator_name = (list_name).begin(),        \
                                   end_name = (list_name).end();               \
       iterator_name != end_name; ++iterator_name)

#define UI_FIRST_WIDGET(list_name)                                             \
  ((list_name).empty() ? nullptr : (list_name).front())

namespace ui
{

class Widget;
using WidgetsList = std::vector<Widget*>;

} // namespace ui
