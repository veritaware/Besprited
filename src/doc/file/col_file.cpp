// Document Library
// Aseprite  | Copyright (C) 2001-2016 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "base/base.h"
#include "base/cfile.h"
#include "base/clamp.h"
#include "doc/color_scales.h"
#include "doc/image.h"
#include "doc/palette.h"

#include <cstdio>
#include <cstdlib>

#define PROCOL_MAGIC_NUMBER 0xB123

namespace doc::file
{

using namespace base;

// Loads a COL file (Animator and Animator Pro format)
std::shared_ptr<Palette> load_col_file(const char* filename)
{
  std::shared_ptr<Palette> pal;
  int c, r, g, b;
  FILE* f;

  f = std::fopen(filename, "rb");
  if (!f)
    return nullptr;

  // Get file size. A non-seekable stream (e.g. a FIFO) makes ftell() fail
  // (-1); treating that as a huge unsigned size would wrap `size - 8` and
  // ultimately feed a bogus color count into Palette::create() below (see
  // issue #219), so reject outright instead.
  if (std::fseek(f, 0, SEEK_END) != 0)
  {
    fclose(f);
    return nullptr;
  }
  const long rawSize = std::ftell(f);
  if (rawSize < 8)
  {
    fclose(f);
    return nullptr;
  }
  const std::size_t size = static_cast<std::size_t>(rawSize);
  // Computed directly on size_t rather than via std::div() (which is
  // int-only) so that files >= 2 GiB don't truncate the color count (#236).
  const std::size_t quot = (size - 8) / 3;
  const std::size_t rem = (size - 8) % 3;
  if (std::fseek(f, 0, SEEK_SET) != 0)
  {
    fclose(f);
    return nullptr;
  }

  const bool pro = (size == 768) ? false : true; // is Animator Pro format?
  if (!(size) || (pro && rem))
  { // Invalid format
    fclose(f);
    return nullptr;
  }

  // Animator format
  if (!pro)
  {
    pal = Palette::create(256);

    for (c = 0; c < 256; c++)
    {
      r = fgetc(f);
      if (feof(f) || ferror(f))
        break;
      g = fgetc(f);
      if (feof(f) || ferror(f))
        break;
      b = fgetc(f);
      if (feof(f) || ferror(f))
        break;

      pal->setEntry(c, rgba(scale_6bits_to_8bits(base::clamp(r, 0, 63)),
                            scale_6bits_to_8bits(base::clamp(g, 0, 63)),
                            scale_6bits_to_8bits(base::clamp(b, 0, 63)), 255));
    }
  }
  // Animator Pro format
  else
  {
    int magic, version;

    fgetl(f);           // Skip file size
    magic = fgetw(f);   // File format identifier
    version = fgetw(f); // Version file

    // Unknown format
    if (magic != PROCOL_MAGIC_NUMBER || version != 0)
    {
      fclose(f);
      return nullptr;
    }

    pal = Palette::create(static_cast<int>(MIN(quot, static_cast<std::size_t>(256))));

    for (c = 0; c < pal->size(); c++)
    {
      r = fgetc(f);
      if (feof(f) || ferror(f))
        break;
      g = fgetc(f);
      if (feof(f) || ferror(f))
        break;
      b = fgetc(f);
      if (feof(f) || ferror(f))
        break;

      pal->setEntry(c, rgba(base::clamp(r, 0, 255), base::clamp(g, 0, 255),
                            base::clamp(b, 0, 255), 255));
    }
  }

  fclose(f);
  return pal;
}

// Saves an Animator Pro COL file
bool save_col_file(const Palette& pal, const char* filename)
{
  FILE* f = fopen(filename, "wb");
  if (!f)
    return false;

  fputl(8 + 768, f);             // File size
  fputw(PROCOL_MAGIC_NUMBER, f); // File format identifier
  fputw(0, f);                   // Version file

  uint32_t c;
  for (int i = 0, max = pal.size(); i < 256; i++)
  {
    c = i < max ? pal.getEntry(i) : 0;
    fputc(rgba_getr(c), f);
    fputc(rgba_getg(c), f);
    fputc(rgba_getb(c), f);
    if (ferror(f))
      break;
  }

  fclose(f);
  return true;
}

} // namespace doc::file
