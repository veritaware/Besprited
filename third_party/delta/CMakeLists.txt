# cmake support for delta
# Delta     | Copyright (C) 2026 Felipe Manga
# Besprited | Copyright (C) 2026 Veritaware

add_library(delta
  JSON.cpp
  QuickJSInterpreter.cpp
)

target_include_directories(delta PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(delta PUBLIC ${QUICKJS_DIR})
set_target_properties(delta PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
# Besprited builds against the quickjs-ng submodule (third_party/quickjs)
# instead of LibreSprite's quickjs-amalgam: the engine is the `qjs` target and
# the std/os helpers delta relies on (js_std_loop & co.) live in `qjs-libc`.
target_link_libraries(delta PUBLIC qjs qjs-libc)
