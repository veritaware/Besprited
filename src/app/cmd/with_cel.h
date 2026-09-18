// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "doc/object_id.h"
#include <memory>

namespace doc
{
class Cel;
}

namespace app::cmd
{
using namespace doc;

class WithCel
{
public:
  explicit WithCel(const std::shared_ptr<Cel>& cel);
  [[nodiscard]] std::shared_ptr<Cel> cel() const;

private:
  ObjectId m_celId;
};

} // namespace app::cmd
