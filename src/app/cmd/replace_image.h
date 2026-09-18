// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "app/cmd.h"
#include "app/cmd/with_sprite.h"
#include "doc/image_ref.h"

namespace app::cmd
{
using namespace doc;

class ReplaceImage : public Cmd,
                     public WithSprite
{
public:
  ReplaceImage(const Sprite* sprite, const ImageRef& oldImage,
               const ImageRef& newImage);

protected:
  void onExecute() override;
  void onUndo() override;
  void onRedo() override;
  [[nodiscard]] size_t onMemSize() const override
  {
    return sizeof(*this) + (m_copy ? m_copy->getMemSize() : 0);
  }

private:
  void replaceImage(ObjectId oldId, const ImageRef& newImage) const;

  ObjectId m_oldImageId;
  ObjectId m_newImageId;

  // Reference used only to keep the copy of the new image from the
  // ReplaceImage() ctor until the ReplaceImage::onExecute() call.
  // Then the reference is not used anymore.
  ImageRef m_newImage;
  ImageRef m_copy;
};

} // namespace app::cmd
