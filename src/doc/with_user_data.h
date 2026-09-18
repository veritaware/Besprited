// Document Library
// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include "doc/object.h"
#include "doc/user_data.h"

namespace doc
{

class WithUserData : public Object
{
public:
  WithUserData(ObjectType type)
    : Object(type)
  {
  }

  const UserData& userData() const { return m_userData; }

  void setUserData(const UserData& userData) { m_userData = userData; }

private:
  UserData m_userData;
};

} // namespace doc
