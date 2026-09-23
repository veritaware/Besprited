// Document Library
// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include "base/shared_ptr.h"
#include "doc/image_ref.h"
#include "doc/object.h"
#include "doc/with_user_data.h"

namespace doc
{

class CelData : public WithUserData
{
public:
  CelData(const ImageRef& image);
  CelData(const CelData& celData);

  const gfx::Point& position() const { return m_position; }
  int opacity() const { return m_opacity; }
  Image* image() const { return const_cast<Image*>(m_image.get()); };
  ImageRef imageRef() const { return m_image; }

  void setImage(const ImageRef& image);
  void setPosition(int x, int y)
  {
    m_position.x = x;
    m_position.y = y;
  }
  void setPosition(const gfx::Point& pos) { m_position = pos; }
  void setOpacity(int opacity) { m_opacity = opacity; }

  virtual size_t getMemSize() const override
  {
    ASSERT(m_image);
    // cppcheck-suppress nullPointerRedundantCheck
    return sizeof(CelData) + m_image->getMemSize();
  }

private:
  ImageRef m_image;
  gfx::Point m_position; // X/Y screen position
  int m_opacity;         // Opacity level
};

typedef base::SharedPtr<CelData> CelDataRef;

} // namespace doc
