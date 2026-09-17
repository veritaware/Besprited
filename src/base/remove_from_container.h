// Base Library
// Aseprite  | Copyright (C) 2001-2013 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

namespace base
{

// Removes all ocurrences of the specified element from the STL container.
template <typename ContainerType>
void remove_from_container(ContainerType& container,
                           typename ContainerType::const_reference element)
{
  for (typename ContainerType::iterator it = container.begin(),
                                        end = container.end();
       it != end;)
  {
    if (*it == element)
    {
      it = container.erase(it);
      end = container.end();
    }
    else
      ++it;
  }
}

} // namespace base
