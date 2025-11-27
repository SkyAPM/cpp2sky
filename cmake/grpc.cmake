# Copyright 2018 gRPC authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# cmake build file for C++ route_guide example.
# Assumes protobuf and gRPC have been installed using cmake.
# See cmake_externalproject/CMakeLists.txt for all-in-one cmake build
# that automatically builds all the dependencies before building route_guide.
 
cmake_minimum_required(VERSION 3.14)
 
if(MSVC)
  add_definitions(-D_WIN32_WINNT=0x600)
endif()
 
find_package(Threads REQUIRED)
# Auto-detect gRPC submodule if the top-level didn't set the option
if(NOT DEFINED GRPC_AS_SUBMODULE)
  if(EXISTS "${CMAKE_SOURCE_DIR}/3rdparty/grpc")
    set(GRPC_AS_SUBMODULE ON CACHE BOOL "Use gRPC as submodule (auto-detected)")
    if(NOT DEFINED GRPC_FETCHCONTENT)
      # Prefer submodule when present: disable FetchContent unless user explicitly requested it
      set(GRPC_FETCHCONTENT OFF CACHE BOOL "Disable FetchContent since submodule is present")
    endif()
  else()
    set(GRPC_AS_SUBMODULE OFF CACHE BOOL "Use gRPC as a git submodule under 3rdparty/grpc")
    if(NOT DEFINED GRPC_FETCHCONTENT)
      # Fallback: enable FetchContent when submodule absent
      set(GRPC_FETCHCONTENT ON CACHE BOOL "Use FetchContent (fallback)")
    endif()
  endif()
endif()

# Sanity check for conflicting options (user-provided flags only)
if(GRPC_AS_SUBMODULE AND GRPC_FETCHCONTENT)
  message(FATAL_ERROR "Conflicting options: GRPC_AS_SUBMODULE and GRPC_FETCHCONTENT are both ON. Choose one.")
endif()
 
if(GRPC_AS_SUBMODULE)
  # One way to build a projects that uses gRPC is to just include the
  # entire gRPC project tree via "add_subdirectory".
  # This approach is very simple to use, but the are some potential
  # disadvantages:
  # * it includes gRPC's CMakeLists.txt directly into your build script
  #   without and that can make gRPC's internal setting interfere with your
  #   own build.
  # * depending on what's installed on your system, the contents of submodules
  #   in gRPC's third_party/* might need to be available (and there might be
  #   additional prerequisites required to build them). Consider using
  #   the gRPC_*_PROVIDER options to fine-tune the expected behavior.
  #
  # A more robust approach to add dependency on gRPC is using
  # cmake's ExternalProject_Add (see cmake_externalproject/CMakeLists.txt).
 
  # Include the gRPC's cmake build (normally grpc source code would live
  # in a git submodule called "third_party/grpc", but this example lives in
  # the same repository as gRPC sources, so we just look a few directories up)
  if(NOT GRPC_ROOT_DIR)
    set(GRPC_ROOT_DIR "${CMAKE_SOURCE_DIR}/3rdparty/grpc")
  endif()
  # When building gRPC as a subdirectory, disable protobuf's install() export
  # and tests by default to avoid protobuf trying to create an install export
  # that references Abseil targets which are not part of the export set.
  if(NOT DEFINED protobuf_INSTALL)
    set(protobuf_INSTALL OFF CACHE BOOL "Disable protobuf install when built as submodule")
  endif()
  if(NOT DEFINED protobuf_BUILD_TESTS)
    set(protobuf_BUILD_TESTS OFF CACHE BOOL "Disable protobuf tests when built as submodule")
  endif()
  add_subdirectory("${GRPC_ROOT_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/grpc")
  message(STATUS "Using gRPC via add_subdirectory.")
  # After using add_subdirectory, we can now use the grpc targets directly from
  # this build.
  set(_PROTOBUF_LIBPROTOBUF libprotobuf)
  set(_REFLECTION grpc++_reflection)
  if(CMAKE_CROSSCOMPILING)
    find_program(_PROTOBUF_PROTOC protoc)
  else()
    set(_PROTOBUF_PROTOC $<TARGET_FILE:protoc>)
  endif()
  set(_GRPC_GRPCPP grpc++)
  if(CMAKE_CROSSCOMPILING)
    find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
  else()
    set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:grpc_cpp_plugin>)
  endif()
elseif(GRPC_FETCHCONTENT)
  # Another way is to use CMake's FetchContent module to clone gRPC at
  # configure time. This makes gRPC's source code available to your project,
  # similar to a git submodule.
  message(STATUS "Using gRPC via FetchContent (git clone).")
  include(FetchContent)

  set(GRPC_GIT_URL https://github.com/grpc/grpc.git)
  set(GRPC_GIT_TAG v1.74.1)

  FetchContent_Declare(
    grpc
    GIT_REPOSITORY ${GRPC_GIT_URL}
    GIT_TAG        ${GRPC_GIT_TAG}
  )

  # Populate the content so we can initialize nested submodules if present,
  # then add_subdirectory from the populated source dir.
  FetchContent_GetProperties(grpc)
  if(NOT grpc_POPULATED)
    FetchContent_Populate(grpc)
    find_package(Git REQUIRED)
    execute_process(
      COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
      WORKING_DIRECTORY ${grpc_SOURCE_DIR}
      RESULT_VARIABLE _grpc_submod_result
      OUTPUT_QUIET
      ERROR_QUIET
    )
    # Same safeguard when populating gRPC via FetchContent: prevent protobuf
    # from registering install exports that reference Abseil-only targets.
    if(NOT DEFINED protobuf_INSTALL)
      set(protobuf_INSTALL OFF CACHE BOOL "Disable protobuf install when built via FetchContent")
    endif()
    if(NOT DEFINED protobuf_BUILD_TESTS)
      set(protobuf_BUILD_TESTS OFF CACHE BOOL "Disable protobuf tests when built via FetchContent")
    endif()
    add_subdirectory(${grpc_SOURCE_DIR} ${grpc_BINARY_DIR})
  endif()

  # Since we used add_subdirectory, we can use the grpc targets directly from
  # this build.
  set(_PROTOBUF_LIBPROTOBUF libprotobuf)
  set(_REFLECTION grpc++_reflection)
  set(_PROTOBUF_PROTOC $<TARGET_FILE:protoc>)
  set(_GRPC_GRPCPP grpc++)
  if(CMAKE_CROSSCOMPILING)
    find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
  else()
    set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:grpc_cpp_plugin>)
  endif()
else()
  # This branch assumes that gRPC and all its dependencies are already installed
  # on this system, so they can be located by find_package().
  message(STATUS "gRPC and all its dependencies should  be able to located by find_package().")
 
  # Find Protobuf installation
  # Looks for protobuf-config.cmake file installed by Protobuf's cmake installation.
  option(protobuf_MODULE_COMPATIBLE TRUE)
  find_package(Protobuf CONFIG REQUIRED)
  message(STATUS "Using protobuf ${Protobuf_VERSION}")
 
  set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
  set(_REFLECTION gRPC::grpc++_reflection)
 
  message(STATUS "CMAKE_CROSSCOMPILING: ${CMAKE_CROSSCOMPILING}")
 
  if(CMAKE_CROSSCOMPILING)
    find_program(_PROTOBUF_PROTOC protoc)
  else()
    set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
  endif()
 
  # Find gRPC installation
  # Looks for gRPCConfig.cmake file installed by gRPC's cmake installation.
  find_package(gRPC CONFIG REQUIRED)
  message(STATUS "Using gRPC ${gRPC_VERSION}")
 
  set(_GRPC_GRPCPP gRPC::grpc++)
  if(CMAKE_CROSSCOMPILING)
    find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
  else()
    set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
  endif()

endif()