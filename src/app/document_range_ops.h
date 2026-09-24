// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "doc/document_range.h"

#include <vector>

namespace app
{
class Document;

enum DocumentRangePlace
{
  kDocumentRangeBefore,
  kDocumentRangeAfter,
};

// These functions returns the new location of the "from" range or
// throws an std::runtime_error() in case that the operation cannot
// be done. (E.g. the background layer cannot be moved.)
doc::DocumentRange move_range(Document* doc, const doc::DocumentRange& from,
                              const doc::DocumentRange& to,
                              DocumentRangePlace place);
doc::DocumentRange copy_range(Document* doc, const doc::DocumentRange& from,
                              const doc::DocumentRange& to,
                              DocumentRangePlace place);

void reverse_frames(Document* doc, const doc::DocumentRange& range);

} // namespace app
