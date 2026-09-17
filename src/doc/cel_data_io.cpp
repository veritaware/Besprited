// Document Library
// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "doc/cel_data_io.h"

#include "base/serialization.h"
#include "doc/cel_data.h"
#include "doc/subobjects_io.h"
#include "doc/user_data_io.h"

#include <iostream>
#include <memory>

namespace doc
{

using namespace base::serialization;
using namespace base::serialization::little_endian;

void write_celdata(std::ostream& os, const CelData* celdata)
{
  write32(os, celdata->id());
  write32(os, static_cast<int16_t>(celdata->position().x));
  write32(os, static_cast<int16_t>(celdata->position().y));
  write8(os, celdata->opacity());
  write32(os, celdata->image()->id());
  write_user_data(os, celdata->userData());
}

CelData* read_celdata(std::istream& is, SubObjectsIO* subObjects, bool setId)
{
  const ObjectId id = read32(is);
  const int x = read32(is);
  const int y = read32(is);
  const int opacity = read8(is);
  const ObjectId imageId = read32(is);
  const UserData userData = read_user_data(is);

  const ImageRef image(subObjects->getImageRef(imageId));
  if (!image)
    return nullptr;

  std::unique_ptr<CelData> celdata = std::make_unique<CelData>(image);
  celdata->setPosition(x, y);
  celdata->setOpacity(opacity);
  celdata->setUserData(userData);
  if (setId)
    celdata->setId(id);
  return celdata.release();
}

} // namespace doc
