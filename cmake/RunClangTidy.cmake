# Besprited | Copyright (C) 2026 Veritaware
#
# Runs clang-tidy over only the source files present in the active
# compile_commands.json, instead of every *.cpp/*.cc/*.c under src/. A blind
# glob picks up files that aren't part of this build configuration at all
# (e.g. she/win or she/gtk sources on a non-Windows/non-GTK build, or
# *_tests.cpp files when ENABLE_TESTS is off) and clang-tidy has no real
# compiler flags for those, so it fails them with spurious "file not found"
# errors instead of reporting real findings.
#
# Invoked via `${CMAKE_COMMAND} -P` from the `clang-tidy` custom target in
# the top-level CMakeLists.txt, which supplies:
#   CLANG_TIDY_EXECUTABLE, COMPILE_COMMANDS_JSON, SRC_DIR, EXTRA_ARGS

cmake_minimum_required(VERSION 3.19)

if(NOT EXISTS "${COMPILE_COMMANDS_JSON}")
  message(FATAL_ERROR
    "compile_commands.json not found at ${COMPILE_COMMANDS_JSON}. "
    "Configure the build first (CMAKE_EXPORT_COMPILE_COMMANDS is enabled by default).")
endif()

file(READ "${COMPILE_COMMANDS_JSON}" JSON_CONTENT)
string(JSON NUM_ENTRIES LENGTH "${JSON_CONTENT}")

set(SOURCE_FILES "")
if(NUM_ENTRIES GREATER 0)
  math(EXPR LAST_INDEX "${NUM_ENTRIES} - 1")
  foreach(i RANGE ${LAST_INDEX})
    string(JSON FILE_PATH GET "${JSON_CONTENT}" ${i} file)
    if(FILE_PATH MATCHES "^${SRC_DIR}/")
      list(APPEND SOURCE_FILES "${FILE_PATH}")
    endif()
  endforeach()
endif()

list(REMOVE_DUPLICATES SOURCE_FILES)
list(LENGTH SOURCE_FILES NUM_FILES)
if(NUM_FILES EQUAL 0)
  message(FATAL_ERROR "No source files under ${SRC_DIR} found in ${COMPILE_COMMANDS_JSON}.")
endif()
message(STATUS "clang-tidy: analyzing ${NUM_FILES} file(s) from the active compile database")

execute_process(
  COMMAND "${CLANG_TIDY_EXECUTABLE}" "-p=${BINARY_DIR}" ${EXTRA_ARGS} ${SOURCE_FILES}
  WORKING_DIRECTORY "${SRC_DIR}/.."
  RESULT_VARIABLE CLANG_TIDY_RESULT
)

if(NOT CLANG_TIDY_RESULT EQUAL 0)
  message(FATAL_ERROR "clang-tidy reported issues (exit code ${CLANG_TIDY_RESULT})")
endif()
