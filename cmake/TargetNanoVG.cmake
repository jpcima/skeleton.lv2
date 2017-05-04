set(NANOVG_SOURCE_DIR
  "${PROJECT_SOURCE_DIR}/thirdparty/nanovg")

option(NANOVG_USE_FREETYPE "Whether NanoVG should use Freetype" OFF)
option(NANOVG_USE_GLEW "Whether NanoVG should use GLEW" ON)

if(NOT NANOVG_GL_VERSION)
  set(NANOVG_GL_VERSION "GL2" CACHE STRING "OpenGL version for NanoVG")
endif()

add_library(nanovg STATIC "${PROJECT_SOURCE_DIR}/cmake/TargetNanoVG_impl.c")
target_include_directories(nanovg PUBLIC "${NANOVG_SOURCE_DIR}/src")

find_package(OpenGL REQUIRED)
target_link_libraries(nanovg ${OPENGL_LIBRARIES})
target_include_directories(nanovg PUBLIC "${OPENGL_INCLUDE_DIR}")

if(NANOVG_GL_VERSION STREQUAL "GL2")
  target_compile_definitions(nanovg
    PUBLIC "NANOVG_GL2=1"
    PRIVATE "NANOVG_GL2_IMPLEMENTATION=1")
elseif(NANOVG_GL_VERSION STREQUAL "GL3")
  target_compile_definitions(nanovg
    PUBLIC "NANOVG_GL3=1"
    PRIVATE "NANOVG_GL3_IMPLEMENTATION=1")
else()
  message(FATAL_ERROR "unknown OpenGL requested version '${NANOVG_GL_VERSION}'")
endif()

if(NANOVG_USE_FREETYPE)
  find_package(Freetype REQUIRED)
  target_compile_definitions(nanovg PRIVATE "FONS_USE_FREETYPE=1")
  target_include_directories(nanovg PRIVATE ${FREETYPE_INCLUDE_DIRS})
  target_link_libraries(nanovg ${FREETYPE_LIBRARIES})
endif()

if(NANOVG_USE_GLEW)
  find_package(GLEW REQUIRED)
  find_package(StaticGLEW)
  target_compile_definitions(nanovg PRIVATE "NANOVG_GLEW=1")
  target_include_directories(nanovg PRIVATE ${GLEW_INCLUDE_DIRS})
  if(StaticGLEW_FOUND)
    target_compile_definitions(nanovg PRIVATE ${StaticGLEW_DEFINITIONS})
    target_link_libraries(nanovg ${StaticGLEW_LIBRARIES})
  else()
    target_link_libraries(nanovg ${GLEW_LIBRARIES})
  endif()
endif()

set_target_properties(nanovg PROPERTIES
  POSITION_INDEPENDENT_CODE ON
  C_VISIBILITY_PRESET hidden)
