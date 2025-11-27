cmake_minimum_required(VERSION 3.14)

if(MSVC)
  add_definitions(-D_WIN32_WINNT=0x600)
endif()
 
find_package(Threads REQUIRED)
## Auto-detect httplib submodule if top-level didn't set the option
if(NOT DEFINED HTTPLIB_AS_SUBMODULE)
  # using submodule in case of git clone timeout
  if(EXISTS "${CMAKE_SOURCE_DIR}/3rdparty/httplib/CMakeLists.txt")
    set(HTTPLIB_AS_SUBMODULE ON CACHE BOOL "Use httplib as submodule (auto-detected)")
    if(NOT DEFINED HTTPLIB_FETCHCONTENT)
      # Prefer submodule when present: disable FetchContent unless user explicitly requested it
      set(HTTPLIB_FETCHCONTENT OFF CACHE BOOL "Disable FetchContent since submodule is present")
    endif()
  else()
    set(HTTPLIB_AS_SUBMODULE OFF CACHE BOOL "Use httplib as a git submodule under 3rdparty/httplib")
    if(NOT DEFINED HTTPLIB_FETCHCONTENT)
      # Fallback: enable FetchContent when submodule absent
      set(HTTPLIB_FETCHCONTENT ON CACHE BOOL "Use FetchContent (fallback)")
    endif()
  endif()
endif()

# Sanity check for conflicting options (user-provided flags only)
if(HTTPLIB_AS_SUBMODULE AND HTTPLIB_FETCHCONTENT)
  message(FATAL_ERROR "Conflicting options: HTTPLIB_AS_SUBMODULE and HTTPLIB_FETCHCONTENT are both ON. Choose one.")
endif()

if(HTTPLIB_AS_SUBMODULE)
  add_subdirectory("${CMAKE_SOURCE_DIR}/3rdparty/httplib" "${CMAKE_CURRENT_BINARY_DIR}/httplib")
  message(STATUS "Using httplib via add_subdirectory.")
elseif(HTTPLIB_FETCHCONTENT)
  # using FetchContent to install spdlog
  include(FetchContent)
  if(${CMAKE_VERSION} VERSION_LESS 3.14)
      include(add_FetchContent_MakeAvailable.cmake)
  endif()  

  set(HTTPLIB_GIT_TAG  v0.22.0)
  set(HTTPLIB_GIT_URL  https://github.com/yhirose/cpp-httplib.git)

  FetchContent_Declare(
    httplib
    GIT_REPOSITORY    ${HTTPLIB_GIT_URL}
    GIT_TAG           ${HTTPLIB_GIT_TAG}
  )  

  FetchContent_MakeAvailable(httplib)
else()
  find_package(httplib CONFIG REQUIRED)
  message(STATUS "Using httplib by find_package")
endif()
