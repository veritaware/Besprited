// Document Library
// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <vector>
#include <memory>

namespace doc
{

class Cel;

typedef std::vector<std::shared_ptr<Cel>> CelList;
typedef CelList::iterator CelIterator;
typedef CelList::const_iterator CelConstIterator;

} // namespace doc
