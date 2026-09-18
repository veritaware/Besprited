// Document Library
// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <vector>

namespace doc
{

class Layer;

typedef std::vector<Layer*> LayerList;
typedef LayerList::iterator LayerIterator;
typedef LayerList::const_iterator LayerConstIterator;

} // namespace doc
