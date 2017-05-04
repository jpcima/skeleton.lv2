#include "framework/ui.h"
#include "meta/project.h"
#include <GL/glew.h>
#include <pugl/gl.h>
#include <pugl/pugl.h>
#include <boost/scope_exit.hpp>
#include <iostream>

struct UI::Impl {
  static constexpr unsigned width = 600;
  static constexpr unsigned height = 400;
  PuglView *view {};
  PuglNativeWindow parent = 0;
  PuglNativeWindow widget = 0;
  bool exposed = false;
  bool initialized_gl = false;
  bool ok_gl = false;
  void create_widget();
  void handle_event(const PuglEvent *event);
  void init_gl();
  void draw_gl();
};

//==============================================================================
UI::UI(void *parent, LV2_URID_Map *map, LV2_URID_Unmap *unmap)
    : P(new Impl) {
  P->parent = PuglNativeWindow(parent);
}

UI::~UI() {
  if (P->view)
    puglDestroy(P->view);
}

LV2UI_Widget UI::widget() const {
  if (!P->widget)
    P->create_widget();
  return LV2UI_Widget(P->widget);
}

unsigned UI::width() {
  return Impl::width;
}

unsigned UI::height() {
  return Impl::height;
}

void UI::port_event(
    uint32_t port_index, uint32_t buffer_size, uint32_t format, const void *buffer) {
}

bool UI::needs_idle_callback() {
  return true;
}

bool UI::idle() {
  PuglView *view = P->view;
  if (!view)
    return false;

  puglProcessEvents(view);

  if (!P->initialized_gl) {
    puglEnterContext(view);
    P->init_gl();
    P->initialized_gl = true;
    puglLeaveContext(view, false);
  }

  if (!P->ok_gl)
    return false;

  puglEnterContext(view);
  P->draw_gl();
  puglLeaveContext(view, true);

  return P->exposed;
}

//==============================================================================
void UI::Impl::create_widget() {
  bool success = false;

  int pugl_argc = 1;
  char pugl_arg0[] = "pugl";
  char *pugl_argv[] = {pugl_arg0, nullptr};

  PuglView *view = puglInit(&pugl_argc, pugl_argv);
  if (!view) {
    std::cerr << "error creating a Pugl view\n";
    return;
  }

  BOOST_SCOPE_EXIT(&success, view) {
    if (!success)
      puglDestroy(view);
  } BOOST_SCOPE_EXIT_END;

  puglSetHandle(view, this);
  puglSetEventFunc(view, [](PuglView *view, const PuglEvent *event) {
    reinterpret_cast<UI::Impl *>(puglGetHandle(view))->handle_event(event); });

  puglInitWindowParent(view, this->parent);
  puglInitWindowSize(view, Impl::width, Impl::height);
  puglInitResizable(view, false);
  puglInitContextType(view, PUGL_GL);

  if (puglCreateWindow(view, PROJECT_DISPLAY_NAME) != 0) {
    std::cerr << "error creating a Pugl window\n";
    return;
  }

  puglShowWindow(view);
  puglProcessEvents(view);

  this->view = view;
  this->widget = puglGetNativeWindow(view);
  success = true;
}

void UI::Impl::handle_event(const PuglEvent *event) {
  switch (event->type) {
    case PUGL_EXPOSE: this->exposed = true; break;
    case PUGL_CLOSE: this->exposed = false; break;
    // handle other events here
    default: break;
  }
}

void UI::Impl::init_gl() {
  if (glewInit() != GLEW_OK) {
    std::cerr << "error initializing GLEW\n";
    return;
  }

  glClearColor(0, 0, 0, 0);
  glViewport(0, 0, Impl::width, Impl::height);

  this->ok_gl = true;
}

void UI::Impl::draw_gl() {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, Impl::width, Impl::height, 0, -10, 10);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glClear(GL_COLOR_BUFFER_BIT);

  glBegin(GL_TRIANGLES);
  {
    glColor3f(0, 0, 1);
    glVertex2f(Impl::width / 2, 0);
    glColor3f(0, 1, 0);
    glVertex2f(0, Impl::height - 1);
    glColor3f(1, 0, 0);
    glVertex2f(Impl::width - 1, Impl::height - 1);
  }
  glEnd();
}
