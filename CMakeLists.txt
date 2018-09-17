# Copyright (c) 2018 by Axway. All Rights Reserved.
# Licensed under the terms of the Apache Public License.
# Please see the LICENSE included with this distribution for details.
cmake_minimum_required(VERSION 3.0.0)

project(HAL)

set(HAL_VERSION 2.0.0)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)

option(HAL_DISABLE_TESTS "Disable compiling the tests" OFF)
option(HAL_RENAME_AXWAYHAL "Rename DLL to AXWAYHAL" OFF)

# Use solution folders.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Build shared library by default
set(LIBRARY_BUILD_TYPE SHARED)

get_filename_component(HAL_CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../cmake ABSOLUTE)

# Define helper functions and macros.
include(${HAL_CMAKE_MODULE_PATH}/internal_utils.cmake)

# Defined in internal_utils.cmake.
config_compiler_and_linker()

set(SOURCE_HAL
  include/HAL/HAL.hpp
  )

set(SOURCE_HAL_detail
  include/HAL/detail/JSBase.hpp
  include/HAL/detail/JSUtil.hpp
  src/detail/JSUtil.cpp
  )

set(SOURCE_JSExport
  include/HAL/JSExport.hpp
  include/HAL/JSExportObject.hpp
  src/JSExportObject.cpp
  )

set(SOURCE_JSClass
  include/HAL/JSClass.hpp
  )
  
set(SOURCE_JSContext
  include/HAL/JSContextGroup.hpp
  src/JSContextGroup.cpp
  include/HAL/JSContext.hpp
  src/JSContext.cpp
  )

set(SOURCE_JSValue
  include/HAL/JSValue.hpp
  src/JSValue.cpp
  include/HAL/JSString.hpp
  src/JSString.cpp
  )

set(SOURCE_JSObject
  include/HAL/JSObject.hpp
  src/JSObject.cpp
  include/HAL/JSArray.hpp
  src/JSArray.cpp
  include/HAL/JSError.hpp
  src/JSError.cpp 
  )
  
source_group(HAL                   FILES ${SOURCE_HAL})
source_group(HAL\\detail           FILES ${SOURCE_HAL_detail})
source_group(HAL\\JSExport         FILES ${SOURCE_JSExport})
source_group(HAL\\JSClass          FILES ${SOURCE_JSClass})
source_group(HAL\\JSContext        FILES ${SOURCE_JSContext})
source_group(HAL\\JSValue          FILES ${SOURCE_JSValue})
source_group(HAL\\JSObject         FILES ${SOURCE_JSObject})

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

add_library(HAL ${LIBRARY_BUILD_TYPE}
  ${SOURCE_HAL}
  ${SOURCE_HAL_detail}
  ${SOURCE_JSExport}
  ${SOURCE_JSClass}
  ${SOURCE_JSContext}
  ${SOURCE_JSValue}
  ${SOURCE_JSObject}
  )

include(GenerateExportHeader)
generate_export_header(HAL)
target_compile_definitions(HAL PRIVATE HAL_EXPORTS)

target_include_directories(HAL PUBLIC
  ${PROJECT_SOURCE_DIR}/include
  )

target_link_libraries(HAL
  chakrart.lib
  )

# Silence this warning when lnking the Debug configuration:
#
# "LINK : warning LNK4075: ignoring '/INCREMENTAL' due to '/OPT:ICF'
# specification ..."
#
# According to "/IGNORE (Ignore Specific Warnings)" (see
# http://msdn.microsoft.com/en-us/library/dn782850.aspx) some linker
# warnings cannot be ignored and LNK4075 is one of them. However,
# according to "/INCREMENTAL (Link Incrementally)" (see
# http://msdn.microsoft.com/en-us/library/4khtbfyf.aspx):
#
# "Most programs can be linked incrementally. However, some changes
# are too great, and some options are incompatible with incremental
# linking. LINK performs a full link if any of the following options
# are specified:
# ...
# /OPT:REF is selected
# /OPT:ICF is selected
# ..."
#
# And according to "/OPT (Optimizations)" (see
# http://msdn.microsoft.com/en-us/library/bxwfs976.aspx):
#
# "If /DEBUG is specified, the default for /OPT is NOREF, and all
# functions are preserved in the image."
#
# As well as:
#
# "... we do not recommend that you use /OPT:ICF in debug builds ..."
#
# Therefore we disable /OPT:REF and /OPT:ICF for the Debug
# configuration.
set_property(TARGET HAL APPEND_STRING PROPERTY LINK_FLAGS_DEBUG "/OPT:NOREF /OPT:NOICF")

# Fix error C4996: 'std::_Copy_impl': Function call with parameters that may be unsafe (needed by boost::split)
set_property(TARGET HAL APPEND_STRING PROPERTY COMPILE_FLAGS " -D_SCL_SECURE_NO_WARNINGS")

set_property(TARGET HAL PROPERTY VERSION ${HAL_VERSION})
set_property(TARGET HAL PROPERTY SOVERSION 0)
set_property(TARGET HAL PROPERTY INTERFACE_HAL_MAJOR_VERSION 0)
set_property(TARGET HAL APPEND PROPERTY
  COMPATIBLE_INTERFACE_STRING HAL_MAJOR_VERSION
  )

if (HAL_RENAME_AXWAYHAL)
  set_target_properties(HAL PROPERTIES OUTPUT_NAME "AXWAYHAL")
endif()

install(TARGETS HAL EXPORT HALTargets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
  )

export(EXPORT HALTargets
  FILE "${CMAKE_BINARY_DIR}/HALTargets.cmake"
)

configure_file(cmake/HALConfig.cmake
  "${CMAKE_BINARY_DIR}/HALConfig.cmake"
  COPYONLY
  )

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_BINARY_DIR}/HALConfigVersion.cmake"
  VERSION ${HAL_VERSION}
  COMPATIBILITY AnyNewerVersion
  )

# Store the current build directory in the CMake user package registry
# so that find_package(HAL) works.
export(PACKAGE HAL)
