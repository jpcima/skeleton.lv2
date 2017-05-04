include(FindPackageHandleStandardArgs)
include(FindStaticLibrary)
include(SelectLibraryConfigurations)

find_package(GLEW)

if(GLEW_FOUND)
  find_static_library(StaticGLEW_LIBRARY_RELEASE NAMES GLEW glew32 glew glew32s PATH_SUFFIXES lib64)
  find_static_library(StaticGLEW_LIBRARY_DEBUG NAMES GLEWd glew32d glewd PATH_SUFFIXES lib64)
  select_library_configurations(StaticGLEW)
endif()

find_package_handle_standard_args(StaticGLEW
  REQUIRED_VARS StaticGLEW_LIBRARY)

if(StaticGLEW_FOUND)
  set(StaticGLEW_INCLUDE_DIRS ${GLEW_INCLUDE_DIRS})
  set(StaticGLEW_LIBRARIES "${StaticGLEW_LIBRARY}")
  set(StaticGLEW_DEFINITIONS "GLEW_STATIC=1")
endif()
