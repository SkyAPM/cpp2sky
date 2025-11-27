cmake_minimum_required(VERSION 3.14)

if(MSVC)
  add_definitions(-D_WIN32_WINNT=0x600)
endif()
 
find_package(Threads REQUIRED)
## Auto-detect fmt submodule if top-level didn't set the option
if(NOT DEFINED FMTLIB_AS_SUBMODULE)
  if(EXISTS "${CMAKE_SOURCE_DIR}/3rdparty/fmt/CMakeLists.txt")
    set(FMTLIB_AS_SUBMODULE ON CACHE BOOL "Use fmt as submodule (auto-detected)")
    if(NOT DEFINED FMTLIB_FETCHCONTENT)
      # Prefer submodule when present: disable FetchContent unless user explicitly requested it
      set(FMTLIB_FETCHCONTENT OFF CACHE BOOL "Disable FetchContent since submodule is present")
    endif()
  else()
    set(FMTLIB_AS_SUBMODULE OFF CACHE BOOL "Use fmt as a git submodule under 3rdparty/fmt")
    if(NOT DEFINED FMTLIB_FETCHCONTENT)
      # Fallback: enable FetchContent when submodule absent
      set(FMTLIB_FETCHCONTENT ON CACHE BOOL "Use FetchContent (fallback)")
    endif()
  endif()
endif()

# Sanity check for conflicting options (user-provided flags only)
if(FMTLIB_AS_SUBMODULE AND FMTLIB_FETCHCONTENT)
  message(FATAL_ERROR "Conflicting options: FMTLIB_AS_SUBMODULE and FMTLIB_FETCHCONTENT are both ON. Choose one.")
endif()

if(FMTLIB_AS_SUBMODULE)
  # using submodule in case of git clone timeout 
  if(CPP2SKY_INSTALL)
    set(FMT_INSTALL ON)
  endif(CPP2SKY_INSTALL)
  add_subdirectory("${CMAKE_SOURCE_DIR}/3rdparty/fmt" "${CMAKE_CURRENT_BINARY_DIR}/fmt")
  message(STATUS "Using fmt via add_subdirectory.")
elseif(FMTLIB_FETCHCONTENT)
  # using FetchContent to install spdlog
  include(FetchContent)
  if(${CMAKE_VERSION} VERSION_LESS 3.14)
      include(add_FetchContent_MakeAvailable.cmake)
  endif()

  set(FMTLIB_GIT_URL https://github.com/fmtlib/fmt.git)
  set(FMTLIB_GIT_TAG 8.1.1)

  FetchContent_Declare(
    fmtlib
    GIT_REPOSITORY ${FMTLIB_GIT_URL}
    GIT_TAG        ${FMTLIB_GIT_TAG}
  )
  FetchContent_MakeAvailable(fmtlib)
else()
  find_package(fmt CONFIG REQUIRED)
  message(STATUS "Using fmt by find_package")
endif()
