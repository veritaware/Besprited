# Copyright (C) 2026 Veritaware
# cmake support for cpp-httplib (vendored, MIT license)

add_library(httplib
  httplib.cpp
)

target_include_directories(httplib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(httplib PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
target_link_libraries(httplib PUBLIC Threads::Threads)

# Winsock2 for the socket layer
if(WIN32)
  target_link_libraries(httplib PUBLIC ws2_32)
endif()

# https support; the build falls back to plain http when OpenSSL is absent
# Homebrew's OpenSSL is keg-only, so CMake does not find it on its own.
if(APPLE AND NOT OPENSSL_ROOT_DIR)
  foreach(prefix /opt/homebrew/opt/openssl@3 /usr/local/opt/openssl@3)
    if(EXISTS ${prefix})
      set(OPENSSL_ROOT_DIR ${prefix})
      break()
    endif()
  endforeach()
endif()
find_package(OpenSSL QUIET)
if(OpenSSL_FOUND)
  target_compile_definitions(httplib PUBLIC CPPHTTPLIB_OPENSSL_SUPPORT)
  target_link_libraries(httplib PUBLIC OpenSSL::SSL OpenSSL::Crypto)
  # the Windows system CA cert store (load_system_certs_on_windows)
  if(WIN32)
    target_link_libraries(httplib PUBLIC crypt32)
  endif()
  # the macOS keychain CA certs (a bundled OpenSSL has no CA store of its own)
  if(APPLE)
    # PRIVATE: the CoreFoundation headers it pulls in clash with base/with_handle.h
    target_compile_definitions(httplib PRIVATE CPPHTTPLIB_USE_CERTS_FROM_MACOSX_KEYCHAIN)
    target_link_libraries(httplib PUBLIC "-framework CoreFoundation" "-framework Security")
  endif()
else()
  message(WARNING "OpenSSL not found: script fetch() on https URLs will fail at runtime")
endif()
