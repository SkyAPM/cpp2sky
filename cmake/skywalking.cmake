cmake_minimum_required(VERSION 3.14)

if(MSVC)
  add_definitions(-D_WIN32_WINNT=0x600)
endif()

find_package(Threads REQUIRED)
## Auto-detect skywalking-data-collect-protocol submodule if top-level didn't set the option
if(NOT DEFINED SKYWALKING_AS_SUBMODULE)
  if(EXISTS "${CMAKE_SOURCE_DIR}/3rdparty/skywalking-data-collect-protocol/CMakeLists.txt" OR EXISTS "${CMAKE_SOURCE_DIR}/3rdparty/skywalking-data-collect-protocol")
    set(SKYWALKING_AS_SUBMODULE ON CACHE BOOL "Use skywalking-data-collect-protocol as submodule (auto-detected)")
    if(NOT DEFINED SKYWALKING_FETCHCONTENT)
      # Prefer submodule when present: disable FetchContent unless user explicitly requested it
      set(SKYWALKING_FETCHCONTENT OFF CACHE BOOL "Disable FetchContent since submodule is present")
    endif()
  else()
    set(SKYWALKING_AS_SUBMODULE OFF CACHE BOOL "Use skywalking-data-collect-protocol as a git submodule under 3rdparty/skywalking-data-collect-protocol")
    if(NOT DEFINED SKYWALKING_FETCHCONTENT)
      # Fallback: enable FetchContent when submodule absent
      set(SKYWALKING_FETCHCONTENT ON CACHE BOOL "Use FetchContent (fallback)")
    endif()
  endif()
endif()

# Sanity check for conflicting options (user-provided flags only)
if(SKYWALKING_AS_SUBMODULE AND SKYWALKING_FETCHCONTENT)
  message(FATAL_ERROR "Conflicting options: SKYWALKING_AS_SUBMODULE and SKYWALKING_FETCHCONTENT are both ON. Choose one.")
endif()

## This module respects variables set by the top-level project:
## - SKYWALKING_FETCHCONTENT : option declared at top-level to allow FetchContent fallback
## - SKYWALKING_PROTOCOL_PATH : explicit path override

if(SKYWALKING_AS_SUBMODULE)
  set(SKYWALKING_PROTOCOL_PATH "${CMAKE_SOURCE_DIR}/3rdparty/skywalking-data-collect-protocol" CACHE PATH "Path to skywalking-data-collect-protocol (from submodule)")
  message(STATUS "Using skywalking-data-collect-protocol from submodule: ${SKYWALKING_PROTOCOL_PATH}")
elseif(SKYWALKING_FETCHCONTENT)
  include(FetchContent)
  if(${CMAKE_VERSION} VERSION_LESS 3.14)
    include(add_FetchContent_MakeAvailable.cmake)
  endif()

  set(SKYWALKING_GIT_TAG v10.3.0)
  set(SKYWALKING_GIT_URL https://github.com/apache/skywalking-data-collect-protocol.git)

  FetchContent_Declare(
    skywalking_protocol
    GIT_REPOSITORY    ${SKYWALKING_GIT_URL}
    GIT_TAG           ${SKYWALKING_GIT_TAG}
  )
  FetchContent_MakeAvailable(skywalking_protocol)

  # FetchContent makes available a variable <name>_SOURCE_DIR
  set(SKYWALKING_PROTOCOL_PATH "${skywalking_protocol_SOURCE_DIR}" CACHE PATH "Path to skywalking-data-collect-protocol (from FetchContent)")
  message(STATUS "Using skywalking-data-collect-protocol via FetchContent: ${SKYWALKING_PROTOCOL_PATH}")
else()
  if(NOT DEFINED SKYWALKING_PROTOCOL_PATH)
    # Leave unset; proto2cpp will error with helpful message if not provided
    message(STATUS "skywalking-data-collect-protocol not auto-detected; set -DSKYWALKING_PROTOCOL_PATH=/path or enable SKYWALKING_FETCHCONTENT.")
  endif()
endif()
