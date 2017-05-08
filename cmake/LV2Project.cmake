
if(ENABLE_PROFILER)
  include(FindPkgConfig)
  pkg_check_modules(PROFILER REQUIRED libprofiler)
endif()

if(IS_DIRECTORY "${PROJECT_SOURCE_DIR}/thirdparty/lv2")
  message(STATUS "Using bundled LV2")
  set(LV2_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/thirdparty/lv2")
else()
  include(FindPkgConfig)
  pkg_check_modules(LV2 lv2 REQUIRED)
endif()

if(IS_DIRECTORY "${PROJECT_SOURCE_DIR}/thirdparty/boost")
  message(STATUS "Using bundled Boost")
  set(Boost_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/thirdparty/boost")
else()
  find_package(Boost REQUIRED)
endif()

configure_file(
  "${PROJECT_SOURCE_DIR}/project.h.in"
  "${PROJECT_SOURCE_DIR}/sources/meta/project.h")

if(NOT CMAKE_CROSSCOMPILING)
  add_executable(makelv2manifest tools/makelv2manifest.cc)
  if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    target_link_libraries(makelv2manifest dl)
  endif()
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
    C_VISIBILITY_PRESET "hidden"
    CXX_VISIBILITY_PRESET "hidden")
  if(NOT CMAKE_CROSSCOMPILING)
    add_custom_target(${name}-manifest ALL
      makelv2manifest $<TARGET_FILE:${name}>
      "${CMAKE_CURRENT_BINARY_DIR}/lv2/${PROJECT_NAME}.lv2")
  endif()
  target_include_directories(${name}
    PRIVATE ${LV2_INCLUDE_DIRS}
    PRIVATE ${Boost_INCLUDE_DIRS})
  if(ENABLE_PROFILER)
    target_include_directories(${name} PRIVATE ${PROFILER_INCLUDE_DIRS})
    target_link_libraries(${name} ${PROFILER_LIBRARIES})
  endif()
  install(DIRECTORY "${PROJECT_BINARY_DIR}/lv2" DESTINATION "lib")
endmacro()

macro(add_lv2_ui name)
  add_library(${name} MODULE
    ${ARGN}
    "${PROJECT_SOURCE_DIR}/sources/framework/lv2ui.cc")
  set_target_properties(${name} PROPERTIES
    PREFIX "" SUFFIX ".ui"
    LIBRARY_OUTPUT_NAME "${PROJECT_NAME}"
    LIBRARY_OUTPUT_DIRECTORY "lv2/${PROJECT_NAME}.lv2"
    C_VISIBILITY_PRESET "hidden"
    CXX_VISIBILITY_PRESET "hidden")
  target_include_directories(${name}
    PRIVATE ${LV2_INCLUDE_DIRS}
    PRIVATE ${Boost_INCLUDE_DIRS})
  if(ENABLE_PROFILER)
    target_include_directories(${name} PRIVATE ${PROFILER_INCLUDE_DIRS})
    target_link_libraries(${name} ${PROFILER_LIBRARIES})
  endif()
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
  include(FindPkgConfig)
  pkg_check_modules(GTK3 gtk+-3.0 REQUIRED)
  add_lv2_ui(${name} ${ARGN})
  target_link_libraries(${name} ${GTK3_LIBRARIES})
  target_include_directories(${name}
    PRIVATE ${GTK3_INCLUDE_DIRS})
endmacro()

macro(add_lv2_gtk2ui name)
  include(FindPkgConfig)
  pkg_check_modules(GTK2 gtk+-2.0 REQUIRED)
  add_lv2_ui(${name} ${ARGN})
  target_link_libraries(${name} ${GTK2_LIBRARIES})
  target_include_directories(${name}
    PRIVATE ${GTK2_INCLUDE_DIRS})
endmacro()

macro(add_lv2_glui name)
  include(TargetPugl)
  find_package(GLEW REQUIRED)
  find_package(StaticGLEW)
  add_lv2_ui(${name} ${ARGN})
  target_include_directories(${name} PRIVATE ${GLEW_INCLUDE_DIRS})
  if(StaticGLEW_FOUND)
    target_compile_definitions(${name} PRIVATE ${StaticGLEW_DEFINITIONS})
    target_link_libraries(${name} ${StaticGLEW_LIBRARIES})
  else()
    target_link_libraries(${name} ${GLEW_LIBRARIES})
  endif()
  target_link_libraries(${name} pugl)
endmacro()

macro(add_lv2_nvgui name)
  include(TargetNanoVG)
  add_lv2_glui(${name} ${ARGN})
  target_link_libraries(${name} nanovg)
endmacro()

macro(add_lv2_tkui name)
  find_package(TCL)
  if(NOT TCLTK_FOUND)
    message(FATAL_ERROR "Tcl/Tk could not be found")
  endif()
  add_lv2_ui(${name} ${ARGN})
  target_include_directories(${name}
    PRIVATE "${TCL_INCLUDE_PATH}"
    PRIVATE "${TK_INCLUDE_PATH}")
  target_link_libraries(${name} "${TCL_LIBRARY}" "${TK_LIBRARY}")
endmacro()
