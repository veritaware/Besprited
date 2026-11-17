// Base Library
// Aseprite  | Copyright (C) 2001-2013, 2015 David Capello
// Besprited | Copyright (C) 2026            Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <string>

namespace base
{
namespace launcher
{

// False when the app runs inside a sandbox that doesn't allow handing off
// to other programs (an MSIX package); callers should disable the feature.
bool is_available();

// True for "http:" / "https:" URLs with a non-empty remainder.
bool is_web_url(const std::string& url);

bool open_url(const std::string& url);
bool open_file(const std::string& file);
bool open_folder(const std::string& file);

} // namespace launcher
} // namespace base
