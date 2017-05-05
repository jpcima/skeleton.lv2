set(PUGL_SOURCE_DIR
  "${PROJECT_SOURCE_DIR}/thirdparty/pugl")

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
  find_library(COCOA_LIBRARY Cocoa)
  add_library(pugl STATIC
    "${PUGL_SOURCE_DIR}/pugl/pugl_osx.m")
  target_link_libraries(pugl "${COCOA_LIBRARY}")
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
  add_library(pugl STATIC
    "${PUGL_SOURCE_DIR}/pugl/pugl_win.cpp")
else()
  find_package(X11 REQUIRED)
  add_library(pugl STATIC
    "${PUGL_SOURCE_DIR}/pugl/pugl_x11.c")
  target_include_directories(pugl PRIVATE "${X11_INCLUDE_DIR}")
  target_link_libraries(pugl ${X11_LIBRARIES})
endif()

target_include_directories(pugl PUBLIC "${PUGL_SOURCE_DIR}")

find_package(OpenGL REQUIRED)
target_link_libraries(pugl ${OPENGL_LIBRARIES})
target_include_directories(pugl PUBLIC "${OPENGL_INCLUDE_DIR}")
target_compile_definitions(pugl PRIVATE "PUGL_HAVE_GL=1")

set_target_properties(pugl PROPERTIES
  POSITION_INDEPENDENT_CODE ON
  C_VISIBILITY_PRESET hidden
  CXX_VISIBILITY_PRESET hidden)
