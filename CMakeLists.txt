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
  sources/effect.cc)

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

set(PD_INIT_FILE "${PROJECT_SOURCE_DIR}/pd.ini")
if(NOT EXISTS "${PD_INIT_FILE}")
  message(FATAL_ERROR "Required file ${PD_INIT_FILE} does not exist")
endif()
add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/lv2/${PROJECT_NAME}.lv2/pd.ini"
  DEPENDS "${PD_INIT_FILE}"
  COMMAND ${CMAKE_COMMAND} -E copy
  "${PD_INIT_FILE}"
  "${CMAKE_CURRENT_BINARY_DIR}/lv2/${PROJECT_NAME}.lv2/pd.ini")
add_custom_target(pd-ini ALL
  DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/lv2/${PROJECT_NAME}.lv2/pd.ini")
