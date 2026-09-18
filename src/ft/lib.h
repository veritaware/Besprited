// FreeType Wrapper
// Aseprite  | Copyright (C) 2016 David Capello
// Besprited | Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include "base/disable_copying.h"
#include "ft/freetype_headers.h"

#include <string>

namespace ft
{

class Lib
{
public:
  Lib() { FT_Init_FreeType(&m_ft); }

  ~Lib()
  {
    if (m_ft)
      FT_Done_FreeType(m_ft);
  }

  operator FT_Library() { return m_ft; }

  FT_Face open(const std::string& filename)
  {
    FT_Open_Args args;
    memset(&args, 0, sizeof(args));
    args.flags = FT_OPEN_PATHNAME;
    args.pathname = const_cast<FT_String*>(filename.c_str());

    FT_Face face = nullptr;
    const FT_Error err = FT_Open_Face(m_ft, &args, 0, &face);
    if (!err)
      FT_Select_Charmap(face, FT_ENCODING_UNICODE);
    return face;
  }

private:
  FT_Library m_ft = nullptr;

  DISABLE_COPYING(Lib);
};

} // namespace ft
