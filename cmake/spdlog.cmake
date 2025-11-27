cmake_minimum_required(VERSION 3.14)

if(MSVC)
  add_definitions(-D_WIN32_WINNT=0x600)
endif()
 
find_package(Threads REQUIRED)
# Auto-detect spdlog submodule if top-level didn't set the option
if(NOT DEFINED SPDLOG_AS_SUBMODULE)
  if(EXISTS "${CMAKE_SOURCE_DIR}/3rdparty/spdlog/CMakeLists.txt")
    set(SPDLOG_AS_SUBMODULE ON CACHE BOOL "Use spdlog as submodule (auto-detected)")
    if(NOT DEFINED SPDLOG_FETCHCONTENT)
      # Prefer submodule when present: disable FetchContent unless user explicitly requested it
      set(SPDLOG_FETCHCONTENT OFF CACHE BOOL "Disable FetchContent since submodule is present")
    endif()
  else()
    set(SPDLOG_AS_SUBMODULE OFF CACHE BOOL "Use spdlog as a git submodule under 3rdparty/spdlog")
    if(NOT DEFINED SPDLOG_FETCHCONTENT)
      # Fallback: enable FetchContent when submodule absent
      set(SPDLOG_FETCHCONTENT ON CACHE BOOL "Use FetchContent (fallback)")
    endif()
  endif()
endif()

# Sanity check for conflicting options (user-provided flags only)
if(SPDLOG_AS_SUBMODULE AND SPDLOG_FETCHCONTENT)
  message(FATAL_ERROR "Conflicting options: SPDLOG_AS_SUBMODULE and SPDLOG_FETCHCONTENT are both ON. Choose one.")
endif()

if(SPDLOG_AS_SUBMODULE)
  # using submodule in case of git clone timeout 
  if(CPP2SKY_INSTALL)
    set(SPDLOG_MASTER_PROJECT ON)
  endif(CPP2SKY_INSTALL)
  add_subdirectory("${CMAKE_SOURCE_DIR}/3rdparty/spdlog" "${CMAKE_CURRENT_BINARY_DIR}/spdlog")
  message(STATUS "Using spdlog via add_subdirectory.")
elseif(SPDLOG_FETCHCONTENT)
  # using FetchContent to install spdlog
  include(FetchContent)
  if(${CMAKE_VERSION} VERSION_LESS 3.14)
      include(add_FetchContent_MakeAvailable.cmake)
  endif()  

  set(SPDLOG_GIT_TAG  v1.10.0)
  set(SPDLOG_GIT_URL  https://github.com/gabime/spdlog.git)

  FetchContent_Declare(
    spdlog
    GIT_REPOSITORY    ${SPDLOG_GIT_URL}
    GIT_TAG           ${SPDLOG_GIT_TAG}
  )  

  FetchContent_MakeAvailable(spdlog)
else()
  find_package(spdlog CONFIG REQUIRED)
  message(STATUS "Using spdlog by find_package")
endif()
