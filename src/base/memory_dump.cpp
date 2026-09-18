// Base Library
// Aseprite  | Copyright (C) 2001-2013, 2015 David Capello
// Besprited | Copyright (C) 2026            Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "base/memory_dump.h"

#ifdef _WIN32
#include "base/memory_dump_win32.h"
#else
#include "base/memory_dump_none.h"
#endif

namespace base
{

MemoryDump::MemoryDump()
  : m_impl(new MemoryDumpImpl)
{
}

MemoryDump::~MemoryDump()
{
  delete m_impl;
}

void MemoryDump::setFileName(const std::string& fileName)
{
  m_impl->setFileName(fileName);
}

} // namespace base
