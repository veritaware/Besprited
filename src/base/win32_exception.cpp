// Base Library
// Aseprite  | Copyright (C) 2001-2013 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "base/win32_exception.h"

#include "base/string.h"

#include <windows.h>

namespace base
{

Win32Exception::Win32Exception(const std::string& msg) throw()
  : Exception()
{
  DWORD errcode = GetLastError();
  LPVOID buf;

  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | // TODO Try to use a TLS buffer
                    FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr, errcode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPWSTR)&buf, 0, nullptr);

  setMessage((msg + "\n" + to_utf8((LPWSTR)buf)).c_str());
  LocalFree(buf);
}

Win32Exception::~Win32Exception() throw()
{
}

} // namespace base
