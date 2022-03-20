# Copyright (c) 2022 midnightBITS
# This file is licensed under MIT license (see LICENSE for details)

cmake_minimum_required (VERSION 3.20)
project (http-router
  VERSION 0.5.1
  DESCRIPTION "Router library based on Boost.Beast"
  LANGUAGES CXX)

set(PROJECT_VERSION_STABILITY "-alpha") # "", or "-alpha", or "-beta", or "-rc.5"
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  message(STATUS "http-router: Standalone")
  set(HTTP_ROUTER_STANDALONE ON)
  set(HTTP_ROUTER_FOLDER "")
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

  # Standalone will be compiled with C++20 to suppport char8_t,
  # clients need at least 17 to compile (for string_view for instance).
  if (MSVC)
    # https://github.com/microsoft/STL/issues/1814
    set(STANDARD 23 CACHE BOOL "Tweak the required standard")
  else()
    set(STANDARD 20 CACHE BOOL "Tweak the required standard")
  endif()
  set(CMAKE_CXX_STANDARD ${STANDARD})
  set(CMAKE_CXX_STANDARD_REQUIRED OFF)
  set(CMAKE_CXX_EXTENSIONS OFF)

  find_package(Python3 COMPONENTS Interpreter REQUIRED)

  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_BINARY_DIR}")
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${PROJECT_BINARY_DIR}")

  find_program(CONAN_COMMAND NAMES conan conan.bat)

  if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/0.17.0/conan.cmake"
                  "${CMAKE_BINARY_DIR}/conan.cmake"
                  EXPECTED_HASH SHA256=3bef79da16c2e031dc429e1dac87a08b9226418b300ce004cc125a82687baeef
                  TLS_VERIFY ON)
  endif()

  set(CONAN_CMAKE_SILENT_OUTPUT ON)
  include(${CMAKE_BINARY_DIR}/conan.cmake)
  if (CMAKE_BUILD_TYPE STREQUAL "")
      _conan_detect_build_type(BUILD_TYPE Debug)
  else()
      _conan_detect_build_type()
  endif()
  conan_cmake_install(PATH_OR_REFERENCE ..
                      BUILD missing
                      SETTINGS "build_type=${_CONAN_SETTING_BUILD_TYPE}")
else()
  message(STATUS "http-router: Subdir")
  set(HTTP_ROUTER_STANDALONE OFF)
  set(HTTP_ROUTER_FOLDER external/http-router/ )
endif()

option(HTTP_ROUTER_TESTING "Compile and/or run self-tests" ${HTTP_ROUTER_STANDALONE})
option(HTTP_ROUTER_EXAMPLES "Compile examples" ${HTTP_ROUTER_STANDALONE})
option(HTTP_ROUTER_INSTALL "Install the application" ${HTTP_ROUTER_STANDALONE})
option(HTTP_ROUTER_WITH_STATIC_FILES "Compiles static-files-filter" ON)
option(HTTP_ROUTER_WITH_CORS "Compiles cors-filter" ON)
option(HTTP_ROUTER_WITH_LOGGER "Compiles logger-filter" ON)
option(HTTP_ROUTER_WITH_HEADERS "Compiles headers-filter" ON)

find_package(fmt REQUIRED)
find_package(GTest REQUIRED)
find_package(Boost REQUIRED)

include(http-router-warnings)

add_subdirectory(libs)
add_subdirectory(examples)

if (HTTP_ROUTER_INSTALL)
  include(prepare_pack)
endif()
