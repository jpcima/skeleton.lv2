# if defined(_WIN32)
#  include <windows.h>
# endif

#if defined(__APPLE__)
# include <OpenGL/gl.h>
# include <OpenGL/glext.h>
#else
# define GL_GLEXT_PROTOTYPES 1
# include <GL/gl.h>
# include <GL/glext.h>
#endif

#include <nanovg.h>
#include <nanovg.c>

#define NANOVG_GL_IMPLEMENTATION 1
#include <nanovg_gl.h>
#include <nanovg_gl_utils.h>
