// Base Library
// Aseprite  | Copyright (C) 2001-2013, 2015 David Capello
// Besprited | Copyright (C) 2026            Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "base/exception.h"

#include <cstdio>
#include <cstring>
#include <cstdarg>

namespace base
{

using namespace std;

Exception::Exception() noexcept = default;

Exception::Exception(const char* format, ...) noexcept
{
  try
  {
    if (!std::strchr(format, '%'))
    {
      m_msg = format;
    }
    else
    {
      va_list ap;
      va_start(ap, format);

      char buf[1024]; // TODO warning buffer overflow
      std::vsnprintf(buf, sizeof(buf), format, ap);
      m_msg = buf;

      va_end(ap);
    }
  }
  catch (...)
  {
    // No throw
  }
}

Exception::Exception(const std::string& msg) noexcept
{
  try
  {
    m_msg = msg;
  }
  catch (...)
  {
    // No throw
  }
}

Exception::~Exception() noexcept = default;

void Exception::setMessage(const char* msg) noexcept
{
  try
  {
    m_msg = msg;
  }
  catch (...)
  {
    // No throw
  }
}

const char* Exception::what() const noexcept
{
  return m_msg.c_str();
}

} // namespace base
