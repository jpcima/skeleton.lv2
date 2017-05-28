cmake_minimum_required(VERSION 3.3)

project(lv2-skeleton LANGUAGES C CXX)
set(PROJECT_DISPLAY_NAME "LV2 example")
set(PROJECT_URI "urn:jpcima:lv2-example")

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

option(USE_DYN_MANIFEST "Create a dynmanifest plugin" ON)
option(ENABLE_PROFILER "Enable the profiler library" OFF)

include(CXXStandard)
include(CXXWarnings)
include(LV2Project)

#===============================================================================
# LV2 Effect target
#===============================================================================
add_lv2_fx(fx
  sources/description.cc
  sources/effect.cc
  sources/utility/pd-miniparser.cc
  sources/utility/pd-common.cc)

#===============================================================================
# LV2 GUI target
#===============================================================================
add_lv2_tkui(ui
  sources/ui.cc)

#===============================================================================
# for Puredata plugins
#===============================================================================
if(NOT USE_DYN_MANIFEST)
  message(FATAL_ERROR "Puredata plugins must be dynmanifest")
endif()
find_package(Boost COMPONENTS filesystem system REQUIRED)
target_link_libraries(fx
  "${Boost_FILESYSTEM_LIBRARY}"
  "${Boost_SYSTEM_LIBRARY}")

include(TargetLibpd)
target_link_libraries(fx libpd)

set(PD_DATA_DIR "${PROJECT_SOURCE_DIR}/puredata")
if(NOT EXISTS "${PD_DATA_DIR}/pd.ini")
  message(FATAL_ERROR "Required file ${PD_DATA_DIR}/pd.ini does not exist")
endif()
add_custom_target(pd-ini ALL
  COMMAND "${CMAKE_COMMAND}" -E copy_directory
  "${PD_DATA_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/lv2/${PROJECT_NAME}.lv2/")
