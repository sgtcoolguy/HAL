# HAL
#
# Copyright (c) 2014 by Appcelerator, Inc. All Rights Reserved.
# Licensed under the terms of the Apache Public License.
# Please see the LICENSE included with this distribution for details.

# Author: Matt Langston
# Created: 2014.09.03 
#
# Try to find JavaScriptCore. Once done this will define:
#  
#  JavaScriptCore_FOUND       - system has JavaScriptCore
#  JavaScriptCore_INCLUDE_DIRS - the include directory
#  JavaScriptCore_LIBRARY_DIR - the directory containing the library
#  JavaScriptCore_LIBRARIES   - link these to use JavaScriptCore
#
# and the following imported targets
#
#     JavaScriptCore::JavaScriptCore
#

find_package(PkgConfig)
pkg_check_modules(PC_JavaScriptCore QUIET JavaScriptCore)

find_path(JavaScriptCore_INCLUDE_DIRS
  NAMES JavaScriptCore/JavaScript.h
  HINTS ${PC_JavaScriptCore_INCLUDE_DIRS} ${PC_JavaScriptCore_INCLUDEDIR}
  PATHS ENV JavaScriptCore_HOME
  PATH_SUFFIXES includes
)

set(JavaScriptCore_ARCH "x86")
if (CMAKE_GENERATOR MATCHES "^Visual Studio .+ ARM$" OR CMAKE_VS_PLATFORM_NAME STREQUAL "ARM")
  set(JavaScriptCore_ARCH "arm")
elseif(CMAKE_GENERATOR MATCHES "^Visual Studio .+ Win64$" OR CMAKE_VS_PLATFORM_NAME STREQUAL "x64")
  set(JavaScriptCore_ARCH "x64")
endif()

find_library(JavaScriptCore_LIBRARIES
  NAMES JavaScriptCore JavaScriptCore-Debug JavaScriptCore-Release
  HINTS ${PC_JavaScriptCore_LIBRARY_DIRS} ${PC_JavaScriptCore_LIBDIR}
  PATHS ENV JavaScriptCore_HOME
  PATH_SUFFIXES ${JavaScriptCore_ARCH}
)

if(NOT JavaScriptCore_LIBRARIES MATCHES ".+-NOTFOUND")
  get_filename_component(JavaScriptCore_LIBRARY_DIR ${JavaScriptCore_LIBRARIES} DIRECTORY)

  # If we found the JavaScriptCore library and we're using a Visual
  # Studio generator, then set Visual Studio to use version of JSC lib matching current Configuration of the build.
  if(CMAKE_GENERATOR MATCHES "^Visual Studio .+")
    string(REGEX REPLACE "-(Debug|Release)" "-$(Configuration)" JavaScriptCore_LIBRARIES ${JavaScriptCore_LIBRARIES})
  endif()

endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(JavaScriptCore DEFAULT_MSG JavaScriptCore_INCLUDE_DIRS JavaScriptCore_LIBRARIES)

# set up the target we can re-use
if(JAVASCRIPTCORE_FOUND AND NOT TARGET JavaScriptCore::JavaScriptCore)
    add_library(JavaScriptCore::JavaScriptCore STATIC IMPORTED)
    set_target_properties(JavaScriptCore::JavaScriptCore PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${JavaScriptCore_INCLUDE_DIRS}"
        IMPORTED_LOCATION_DEBUG "${JavaScriptCore_LIBRARY_DIR}/JavaScriptCore-Debug.lib"
        IMPORTED_LOCATION_RELEASE "${JavaScriptCore_LIBRARY_DIR}/JavaScriptCore-Release.lib"
    )
endif()
