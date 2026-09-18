// Base Library
// Aseprite  | Copyright (C) 2001-2013 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include "base/disable_copying.h"

namespace base
{

class mutex
{
public:
  mutex();
  ~mutex();

  void lock();
  bool try_lock();
  void unlock();

private:
  class mutex_impl;
  mutex_impl* m_impl;

  DISABLE_COPYING(mutex);
};

} // namespace base
