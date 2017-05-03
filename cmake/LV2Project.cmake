
include(FindPkgConfig)

pkg_check_modules(LV2 lv2 REQUIRED)
find_package(Boost REQUIRED)

configure_file(
  "${PROJECT_SOURCE_DIR}/project.h.in"
  "${PROJECT_SOURCE_DIR}/sources/meta/project.h")

add_executable(makelv2manifest tools/makelv2manifest.cc)
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  target_link_libraries(makelv2manifest dl)
endif()

macro(add_lv2_fx name)
add_library(${name} MODULE
  ${ARGN}
  "${PROJECT_SOURCE_DIR}/sources/framework/lv2manifest.cc"
  "${PROJECT_SOURCE_DIR}/sources/framework/lv2plugin.cc")
set_target_properties(${name} PROPERTIES
  PREFIX "" SUFFIX ".fx"
  LIBRARY_OUTPUT_NAME "${PROJECT_NAME}"
  LIBRARY_OUTPUT_DIRECTORY "lv2/${PROJECT_NAME}.lv2"
  CXX_VISIBILITY_PRESET "hidden")
add_custom_target(${name}-manifest ALL
  makelv2manifest $<TARGET_FILE:${name}>
  "${CMAKE_CURRENT_BINARY_DIR}/lv2/${PROJECT_NAME}.lv2")
target_include_directories(${name}
  PRIVATE ${LV2_INCLUDE_DIRS}
  PRIVATE "${Boost_INCLUDE_DIR}")
endmacro()

macro(add_lv2_ui name)
add_library(${name} MODULE
  ${ARGN}
  "${PROJECT_SOURCE_DIR}/sources/framework/lv2ui.cc")
set_target_properties(${name} PROPERTIES
  PREFIX "" SUFFIX ".ui"
  LIBRARY_OUTPUT_NAME "${PROJECT_NAME}"
  LIBRARY_OUTPUT_DIRECTORY "lv2/${PROJECT_NAME}.lv2"
  CXX_VISIBILITY_PRESET "hidden")
target_include_directories(${name}
  PRIVATE ${LV2_INCLUDE_DIRS}
  PRIVATE "${Boost_INCLUDE_DIR}")
endmacro()

macro(add_lv2_qt5ui name)
  find_package(Qt5Widgets REQUIRED)
  add_lv2_ui(${name} ${ARGN})
  target_link_libraries(${name} Qt5::Widgets)
endmacro()

macro(add_lv2_qt4ui name)
  find_package(Qt4 REQUIRED QtGui)
  add_lv2_ui(${name} ${ARGN})
  target_link_libraries(${name} Qt4::QtGui)
endmacro()

macro(add_lv2_gtk3ui name)
  pkg_check_modules(GTK3 gtk+-3.0 REQUIRED)
  add_lv2_ui(${name} ${ARGN})
  target_link_libraries(${name} ${GTK3_LIBRARIES})
  target_include_directories(${name}
    PRIVATE ${GTK3_INCLUDE_DIRS})
endmacro()

macro(add_lv2_gtk2ui name)
  pkg_check_modules(GTK2 gtk+-2.0 REQUIRED)
  add_lv2_ui(${name} ${ARGN})
  target_link_libraries(${name} ${GTK2_LIBRARIES})
  target_include_directories(${name}
    PRIVATE ${GTK2_INCLUDE_DIRS})
endmacro()
