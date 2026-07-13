# Scripting Library
# Aseprite    | Copyright (C) 2001-2016 David Capello
# LibreSprite | Copyright (C) 2021-2026 LibreSprite contributors
# Besprited   | Copyright (C) 2026      Veritaware

include_directories(${QUICKJS_DIR})

# Debug C/C++ flags
if(CMAKE_BUILD_TYPE STREQUAL Debug)
  if(LINUX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Og")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Og")
  endif()
else()
  if (LINUX OR APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")
  endif()
endif()

set(SCRIPT_LIB_SOURCES
  quickjs/engine.cpp
  cout_delegate.cpp)

add_library(script-lib ${SCRIPT_LIB_SOURCES})

target_link_libraries(script-lib qjs)
