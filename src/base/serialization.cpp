// Base Library
// Aseprite  | Copyright (C) 2001-2013 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "base/serialization.h"

#include <iostream>

namespace base::serialization
{

std::ostream& write8(std::ostream& os, uint8_t byte)
{
  os.put(byte);
  return os;
}

uint8_t read8(std::istream& is)
{
  return static_cast<uint8_t>(is.get());
}

std::ostream& little_endian::write16(std::ostream& os, uint16_t word)
{
  os.put(word & 0x00ff);
  os.put((word & 0xff00) >> 8);
  return os;
}

std::ostream& little_endian::write32(std::ostream& os, uint32_t dword)
{
  os.put(static_cast<int>(dword & 0x000000ffl));
  os.put(static_cast<int>((dword & 0x0000ff00l) >> 8));
  os.put(static_cast<int>((dword & 0x00ff0000l) >> 16));
  os.put(static_cast<int>((dword & 0xff000000l) >> 24));
  return os;
}

uint16_t little_endian::read16(std::istream& is)
{
  int b1, b2;
  b1 = is.get();
  b2 = is.get();
  return ((b2 << 8) | b1);
}

uint32_t little_endian::read32(std::istream& is)
{
  int b1, b2, b3, b4;
  b1 = is.get();
  b2 = is.get();
  b3 = is.get();
  b4 = is.get();
  return ((b4 << 24) | (b3 << 16) | (b2 << 8) | b1);
}

std::ostream& big_endian::write16(std::ostream& os, uint16_t word)
{
  os.put((word & 0xff00) >> 8);
  os.put(word & 0x00ff);
  return os;
}

std::ostream& big_endian::write32(std::ostream& os, uint32_t dword)
{
  os.put(static_cast<int>((dword & 0xff000000l) >> 24));
  os.put(static_cast<int>((dword & 0x00ff0000l) >> 16));
  os.put(static_cast<int>((dword & 0x0000ff00l) >> 8));
  os.put(static_cast<int>(dword & 0x000000ffl));
  return os;
}

uint16_t big_endian::read16(std::istream& is)
{
  int b1, b2;
  b2 = is.get();
  b1 = is.get();
  return ((b2 << 8) | b1);
}

uint32_t big_endian::read32(std::istream& is)
{
  int b1, b2, b3, b4;
  b4 = is.get();
  b3 = is.get();
  b2 = is.get();
  b1 = is.get();
  return ((b4 << 24) | (b3 << 16) | (b2 << 8) | b1);
}

} // namespace base::serialization
