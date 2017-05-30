#include "framework/ui.h"
#include "meta/project.h"
#include <GL/glew.h>
#include <pugl/gl.h>
#include <pugl/pugl.h>
#include <nanovg.h>
#include <nanovg_gl.h>
#include <boost/scope_exit.hpp>
#include <stdexcept>
#include <iostream>
#include <cmath>

struct UI::Impl {
  static constexpr unsigned width = 600;
  static constexpr unsigned height = 400;
  PuglView *view {};
  PuglNativeWindow parent = 0;
  PuglNativeWindow widget = 0;
  NVGcontext *vg {};
  bool exposed = false;
  bool initialized_nvg = false;
  bool needs_redraw = true;
  void create_widget();
  void handle_event(const PuglEvent *event);
  void init_nvg();
  void draw_nvg();
  void update();
  static void print_gl_info(std::ostream &os);
};

//==============================================================================
UI::UI(void *parent, LV2_URID_Map *map, LV2_URID_Unmap *unmap,
       const char *bundle_path)
    : P(new Impl) {
  P->parent = PuglNativeWindow(parent);
}

UI::~UI() {
  if (P->vg)
    nvgDeleteGL2(P->vg);
  if (P->view)
    puglDestroy(P->view);
}

void UI::option(const LV2_Options_Option &option) {
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
  if (!P->exposed)
    return false;

  if (!P->initialized_nvg) {
    puglEnterContext(view);
    P->init_nvg();
    P->initialized_nvg = true;
    puglLeaveContext(view, false);
  }

  NVGcontext *vg = P->vg;
  if (!vg)
    return false;

  if (P->needs_redraw) {
    puglEnterContext(view);
    P->draw_nvg();
    puglLeaveContext(view, true);
    P->needs_redraw = false;
  }
  return true;
}

//==============================================================================
void UI::Impl::create_widget() {
  bool success = false;

  int pugl_argc = 1;
  char pugl_arg0[] = "pugl";
  char *pugl_argv[] = {pugl_arg0, nullptr};

  PuglView *view = puglInit(&pugl_argc, pugl_argv);
  if (!view)
    throw std::runtime_error("error creating a Pugl view");

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

  if (puglCreateWindow(view, PROJECT_DISPLAY_NAME) != 0)
    throw std::runtime_error("error creating a Pugl window");

  puglShowWindow(view);
  puglProcessEvents(view);

  this->view = view;
  this->widget = puglGetNativeWindow(view);
  success = true;
}

void UI::Impl::handle_event(const PuglEvent *event) {
  switch (event->type) {
    case PUGL_EXPOSE: this->exposed = true; this->needs_redraw = true; break;
    case PUGL_CLOSE: this->exposed = false; break;
    // handle other events here, invoke update() to make the screen redraw
    default: break;
  }
}

#include "../resources/Roboto-Regular.ttf.c"
#include "../resources/Roboto-Bold.ttf.c"

void UI::Impl::init_nvg() {
  if (glewInit() != GLEW_OK) {
    std::cerr << "error initializing GLEW\n";
    return;
  }

  NVGcontext *vg = this->vg = nvgCreateGL2(NVG_ANTIALIAS|NVG_STENCIL_STROKES);
  if (!vg) {
    std::cerr << "error creating a NanoVG context\n";
    return;
  }

  Impl::print_gl_info(std::cerr);

  nvgCreateFontMem(
      vg, "sans",
      const_cast<unsigned char *>(Roboto_Regular_ttf), sizeof(Roboto_Regular_ttf),
      false);
  nvgCreateFontMem(
      vg, "sans-bold",
      const_cast<unsigned char *>(Roboto_Bold_ttf), sizeof(Roboto_Bold_ttf),
      false);
}

void UI::Impl::draw_nvg() {
  glViewport(0, 0, Impl::width, Impl::height);
  glClearColor(0, 0, 0, 0);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

  NVGcontext *vg = this->vg;
  nvgBeginFrame(vg, Impl::width, Impl::height, 1);

  // text drawing example
  {
    nvgSave(vg);

    float w = 400, h = 100;
    float x = (Impl::width - w) / 2, y = 50;

    nvgStrokeColor(vg, nvgRGB(200, 200, 200));
    nvgFillColor(vg, nvgRGB(100, 100, 100));
    nvgStrokeWidth(vg, 10);

    nvgBeginPath(vg);
    nvgRoundedRect(vg, x, y, w, h, 20);
    nvgFill(vg);
    nvgStroke(vg);

    nvgFillColor(vg, nvgRGB(200, 200, 200));

    nvgFontSize(vg, 64);
    nvgFontFace(vg, "sans-bold");
    nvgTextAlign(vg, NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);
    nvgTextBox(vg, x, y + h / 2, w, "Hello, LV2!", nullptr);

    nvgRestore(vg);
  }

  // plot drawing example
  {
    nvgSave(vg);

    constexpr float pi = M_PI;

    float w = Impl::width * 0.9f, h = 150;
    float x = (Impl::width - w) / 2, y = 200;

    constexpr unsigned nsamples = 64;
    float samples[nsamples];
    float sx[nsamples], sy[nsamples];
    float dx = w / (nsamples - 1);

    for (unsigned i = 0; i < nsamples; ++i) {
      float x = 4 * ((2 * i / float(nsamples - 1)) - 1);
      samples[i] = (x == 0) ? 1 : (std::sin(pi * x) / (pi * x));
    }

    for (unsigned i = 0; i < nsamples; ++i) {
      float v = (samples[i] + 0.25f) / 1.25f;
      sx[i] = x + i * dx;
      sy[i] = y + h * (1 - v);
    }

    nvgBeginPath(vg);
    nvgRect(vg, x - 8, y - 8, w + 16, h + 16);
    nvgFillColor(vg, nvgRGB(50, 50, 50));
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgMoveTo(vg, sx[0], sy[0]);
    for (unsigned i = 1; i < nsamples; i++)
      nvgQuadTo(vg, (sx[i] + sx[i-1]) / 2, (sy[i] + sy[i-1]) / 2, sx[i], sy[i]);
    nvgStrokeColor(vg, nvgRGB(255, 200, 0));
    nvgStrokeWidth(vg, 3.0f);
    nvgStroke(vg);

    nvgRestore(vg);
  }

  nvgEndFrame(vg);
}

void UI::Impl::update() {
  this->needs_redraw = true;
}

void UI::Impl::print_gl_info(std::ostream &os) {
  static const struct GLInfo {
    const char *name {};
    GLenum id;
  } infos[] = {
    {"vendor", GL_VENDOR},
    {"renderer", GL_RENDERER},
    {"version", GL_VERSION},
  };
  for (const GLInfo &info: infos) {
    const char *data = reinterpret_cast<const char *>(glGetString(info.id));
    os << "OpenGL " << info.name << ": " << (data ? data : "(unknown)") << "\n";
  }
}
