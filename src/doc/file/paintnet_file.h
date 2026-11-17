// Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <memory>

namespace doc
{
class Palette;

namespace file
{

std::shared_ptr<Palette> load_paintnet_file(const char* filename);
bool save_paintnet_file(const Palette& pal, const char* filename);

} // namespace file
} // namespace doc
