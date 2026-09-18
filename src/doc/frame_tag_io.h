// Document Library
// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <iosfwd>

namespace doc
{

class FrameTag;

void write_frame_tag(std::ostream& os, const FrameTag* tag);
FrameTag* read_frame_tag(std::istream& is, bool setId = true);

} // namespace doc
