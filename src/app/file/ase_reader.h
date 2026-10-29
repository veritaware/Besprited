// Copyright (C) 2026 Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "base/exception.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <string>

namespace app
{

// Borrows the stream for one chunk; reads cannot consume the following chunk.
class AseReader
{
public:
  AseReader(FILE* file, size_t size)
    : m_file(file)
    , m_remaining(size)
  {
  }
  AseReader(const AseReader&) = delete;
  AseReader& operator=(const AseReader&) = delete;

  size_t remaining() const { return m_remaining; }

  long position() const
  {
    const long pos = ftell(m_file);
    if (pos < 0)
      throw base::Exception("Cannot determine ASE input position.");
    return pos;
  }

  void require(size_t size) const
  {
    if (size > m_remaining)
      throw base::Exception("ASE chunk payload is truncated.");
  }

  void read(void* buffer, size_t size)
  {
    require(size);
    if (size && fread(buffer, 1, size, m_file) != size)
      throw base::Exception("Incomplete read of ASE chunk payload.");
    m_remaining -= size;
  }

  uint8_t byte()
  {
    require(1);
    const int value = fgetc(m_file);
    if (value == EOF)
      throw base::Exception("Incomplete read of ASE chunk payload.");
    --m_remaining;
    return static_cast<uint8_t>(value);
  }

  uint16_t word()
  {
    uint8_t bytes[2];
    read(bytes, sizeof(bytes));
    return uint16_t(bytes[0]) | (uint16_t(bytes[1]) << 8);
  }

  uint32_t dword()
  {
    uint8_t bytes[4];
    read(bytes, sizeof(bytes));
    return uint32_t(bytes[0]) | (uint32_t(bytes[1]) << 8) | (uint32_t(bytes[2]) << 16) | (uint32_t(bytes[3]) << 24);
  }

  void skip(size_t size)
  {
    uint8_t buffer[256];
    while (size)
    {
      const size_t count = std::min(size, sizeof(buffer));
      read(buffer, count);
      size -= count;
    }
  }

  std::string string()
  {
    const size_t length = word();
    require(length);
    std::string result(length, '\0');
    read(result.data(), length);
    return result;
  }

private:
  FILE* m_file;
  size_t m_remaining;
};

} // namespace app
