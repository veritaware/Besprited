// SHE Library
// Aseprite  | Copyright (C) 2015 David Capello
// Besprited | Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include "she/native_dialogs.h"

namespace she
{

class NativeDialogsOSX : public NativeDialogs
{
public:
  NativeDialogsOSX();
  FileDialog* createFileDialog() override;
};

} // namespace she
