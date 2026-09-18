// SHE Library
// Aseprite  | Copyright (C) 2012-2014 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include "gfx/fwd.h"

#include <cstdint>

namespace she
{

enum NativeCursor : std::uint8_t
{
  kNoCursor,
  kArrowCursor,
  kIBeamCursor,
  kWaitCursor,
  kLinkCursor,
  kHelpCursor,
  kForbiddenCursor,
  kMoveCursor,
  kSizeNSCursor,
  kSizeWECursor,
  kSizeNCursor,
  kSizeNECursor,
  kSizeECursor,
  kSizeSECursor,
  kSizeSCursor,
  kSizeSWCursor,
  kSizeWCursor,
  kSizeNWCursor,
};

} // namespace she
