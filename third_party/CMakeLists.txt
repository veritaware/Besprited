# Besprited
# Copyright (C) 2001-2016 David Capello
# Copyright (C) 2026      Veritaware

include_directories(.)

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif(MSVC)

add_subdirectory(simpleini)
add_subdirectory(modp_b64)
add_subdirectory(evalmath)

# We only consume QuickJS as a static library (linked into besprited); its CLI
# tools and headers must not land in our install tree / .tar.gz package.
set(QJS_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(quickjs)

# Disable observable tests
set(OBSERVABLE_TESTS OFF CACHE BOOL "Compile observable tests")
add_subdirectory(observable)
include_directories(observable)
