// Document Library
// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include "doc/frame.h"

namespace doc
{

class FrameTag;
class Sprite;

frame_t calculate_next_frame(const Sprite* sprite, frame_t frame,
                             frame_t frameDelta, const FrameTag* tag,
                             bool& pingPongForward);

} // namespace doc
