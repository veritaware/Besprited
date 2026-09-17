// Aseprite  | Copyright (C) 2016 David Capello
// Besprited | Copyright (C) 2026 Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

namespace app::tools
{

enum class [[clang::flag_enum]] ToolLoopModifiers
{
  kNone = 0x00000000,
  kReplaceSelection = 0x00000001,
  kAddSelection = 0x00000002,
  kSubtractSelection = 0x00000004,
  kMoveOrigin = 0x00000008,
  kSquareAspect = 0x00000010,
  kFromCenter = 0x00000020,
};

} // namespace app::tools
