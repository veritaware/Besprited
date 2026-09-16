// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "doc/object_id.h"

namespace app
{
class Document;
}

namespace app::cmd
{

class WithDocument
{
public:
  explicit WithDocument(const Document* doc);
  [[nodiscard]] Document* document() const;

private:
  doc::ObjectId m_docId;
};

} // namespace app::cmd
