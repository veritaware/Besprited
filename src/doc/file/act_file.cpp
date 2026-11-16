// Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "base/fstream_path.h"
#include "doc/color.h"
#include "doc/palette.h"

#include <cstdint>
#include <fstream>
#include <iterator>
#include <memory>
#include <vector>

namespace doc::file
{

namespace
{

constexpr size_t kActColorBytes = 256 * 3;
constexpr uint16_t kNoTransparentIndex = 0xffff;

uint16_t read_be16(const std::vector<uint8_t>& bytes, size_t offset)
{
  return static_cast<uint16_t>((static_cast<uint16_t>(bytes[offset]) << 8) | bytes[offset + 1]);
}

void write_be16(std::ostream& out, uint16_t value)
{
  out.put(static_cast<char>((value >> 8) & 0xff));
  out.put(static_cast<char>(value & 0xff));
}

} // namespace

std::shared_ptr<Palette> load_act_file(const char* filename, int* transparentIndexOut)
{
  if (transparentIndexOut)
    *transparentIndexOut = -1;

  std::ifstream f(FSTREAM_PATH(filename), std::ios::binary);
  if (!f)
    return {};

  const std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
  if (f.bad() || (bytes.size() != kActColorBytes && bytes.size() != kActColorBytes + 4))
    return {};

  int count = 256;
  uint16_t transparentIndex = kNoTransparentIndex;
  if (bytes.size() == kActColorBytes + 4)
  {
    const uint16_t encodedCount = read_be16(bytes, kActColorBytes);
    count = encodedCount == 0 ? 256 : encodedCount;
    transparentIndex = read_be16(bytes, kActColorBytes + 2);
    if (count > 256 || (transparentIndex != kNoTransparentIndex && transparentIndex >= count))
      return {};
  }

  auto pal = Palette::create(count);
  for (int i = 0; i < count; ++i)
  {
    const size_t offset = static_cast<size_t>(i) * 3;
    pal->setEntry(i, rgba(bytes[offset], bytes[offset + 1], bytes[offset + 2], 255));
  }
  if (transparentIndexOut && transparentIndex != kNoTransparentIndex)
    *transparentIndexOut = transparentIndex;
  return pal;
}

bool save_act_file(const Palette& pal, const char* filename)
{
  if (pal.size() <= 0 || pal.size() > 256)
    return false;

  int transparentIndex = -1;
  for (int i = 0; i < pal.size(); ++i)
  {
    const uint8_t alpha = rgba_geta(pal.entry(i));
    if (alpha != 0 && alpha != 255)
      return false;
    if (alpha == 0 && transparentIndex < 0)
      transparentIndex = i;
    else if (alpha == 0)
      return false;
  }

  std::ofstream f(FSTREAM_PATH(filename), std::ios::binary);
  if (!f)
    return false;

  for (int i = 0; i < 256; ++i)
  {
    const color_t color = i < pal.size() ? pal.entry(i) : rgba(0, 0, 0, 255);
    f.put(static_cast<char>(rgba_getr(color)));
    f.put(static_cast<char>(rgba_getg(color)));
    f.put(static_cast<char>(rgba_getb(color)));
  }

  if (pal.size() < 256 || transparentIndex >= 0)
  {
    write_be16(f, pal.size() == 256 ? 0 : static_cast<uint16_t>(pal.size()));
    write_be16(f, transparentIndex < 0 ? kNoTransparentIndex : static_cast<uint16_t>(transparentIndex));
  }
  return f.good();
}

} // namespace doc::file
