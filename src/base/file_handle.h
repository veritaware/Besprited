// Base Library
// Aseprite  | Copyright (C) 2001-2013, 2015 David Capello
// Besprited | Copyright (C) 2026            Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <cstdio>
#include <memory>
#include <string>

namespace base
{

using FileHandle = std::shared_ptr<FILE>;

FILE* open_file_raw(const std::string& filename, const std::string& mode);
FileHandle open_file(const std::string& filename, const std::string& mode);
FileHandle open_file_with_exception(const std::string& filename,
                                    const std::string& mode);
int open_file_descriptor_with_exception(const std::string& filename,
                                        const std::string& mode);

} // namespace base
