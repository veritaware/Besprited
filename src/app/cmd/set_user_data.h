// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "app/cmd.h"
#include "doc/object_id.h"
#include "doc/user_data.h"

namespace doc
{
class WithUserData;
}

namespace app::cmd
{

class SetUserData : public Cmd
{
public:
  SetUserData(const doc::WithUserData* obj, doc::UserData userData);

protected:
  void onExecute() override;
  void onUndo() override;
  [[nodiscard]] size_t onMemSize() const override
  {
    return sizeof(*this) + m_oldUserData.size() + m_newUserData.size();
  }

private:
  doc::ObjectId m_objId;
  doc::UserData m_oldUserData;
  doc::UserData m_newUserData;
};

} // namespace app::cmd
