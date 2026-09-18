// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/cmd/with_document.h"

#include "app/document.h"

namespace app::cmd
{

WithDocument::WithDocument(const Document* doc)
  : m_docId(doc->id())
{
}

Document* WithDocument::document() const
{
  return doc::get<Document>(m_docId);
}

} // namespace app::cmd
