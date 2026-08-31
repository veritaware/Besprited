// Aseprite    | Copyright (C) 2001-2016 David Capello
// LibreSprite | Copyright (C) 2016-2026 LibreSprite contributors
// Besprited   | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef __ASE_CONFIG_H
#error You cannot use config.h two times
#endif

#define __ASE_CONFIG_H

// In MSVC
#ifdef _MSC_VER
  // Avoid warnings about insecure standard C++ functions
  #ifndef _CRT_SECURE_NO_WARNINGS
  #define _CRT_SECURE_NO_WARNINGS
  #endif

  // Disable warning C4355 in MSVC: 'this' used in base member initializer list
  #pragma warning(disable:4355)
#endif

// General information
#define PACKAGE "Besprited"
#define COMMIT  "local build"
#ifndef RELEASE_TAG
    #define VERSION "1.26-dev (" COMMIT ")"
#else
    #define VERSION "1.26.09"
#endif
#define PACKAGE_AND_VERSION PACKAGE " " VERSION

#define WEBSITE                 "https://github.com/Veritaware/Besprited/"
#define WEBSITE_DOWNLOAD        WEBSITE "releases/"
#define WEBSITE_CONTRIBUTORS    WEBSITE "graphs/contributors/"
#define COPYRIGHT               "Copyright © 2001-2016 David Capello, "\
                                            "2016-2026 LibreSprite contributors, "\
                                            "2026 Veritaware"

#include "base/base.h"
#include "base/debug.h"
#include "base/log.h"
