cmake_minimum_required(VERSION 3.3)

project(lv2-skeleton LANGUAGES C CXX)
set(PROJECT_DISPLAY_NAME "LV2 example")
set(PROJECT_URI "urn:jpcima:lv2-example")

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

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
add_lv2_qt5ui(ui
  sources/ui.cc)
