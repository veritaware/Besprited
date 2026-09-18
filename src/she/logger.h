// SHE Library
// Aseprite  | Copyright (C) 2012-2014 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

namespace she
{

class Logger
{
public:
  virtual ~Logger() {}
  virtual void logError(const char* error) = 0;
};

} // namespace she
