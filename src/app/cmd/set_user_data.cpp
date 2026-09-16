// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/cmd/set_user_data.h"

#include <utility>

#include "doc/with_user_data.h"

namespace app::cmd
{

SetUserData::SetUserData(const doc::WithUserData* obj, doc::UserData userData)
  : m_objId(obj->id())
  , m_oldUserData(obj->userData())
  , m_newUserData(std::move(userData))
{
}

void SetUserData::onExecute()
{
  doc::get<doc::WithUserData>(m_objId)->setUserData(m_newUserData);
}

void SetUserData::onUndo()
{
  doc::get<doc::WithUserData>(m_objId)->setUserData(m_oldUserData);
}

} // namespace app::cmd
