#include "framework/ui.h"
#include "meta/project.h"
#include "utility/pd-patchinfo.h"
#include "utility/scope-guard.h"
#include <tcl.h>
#include <tk.h>
#if defined(_WIN32)
# include <tkWin.h>
#endif
#include <iostream>

struct UI::Impl {
  Tcl_Interp *interp = nullptr;
  void *widget = nullptr;
  void *parent = nullptr;
  void create_widget();
  void tk_init(const std::string *args, unsigned count);
  void tk_exec();
  static uintptr_t find_widget(Tcl_Interp *interp, uintptr_t parent);
  static uintptr_t parent_window(Display *dpy, uintptr_t w);
};

//==============================================================================
UI::UI(void *parent, LV2_URID_Map *map, LV2_URID_Unmap *unmap)
    : P(new Impl) {
  P->parent = parent;
}

UI::~UI() {
  Tcl_DeleteInterp(P->interp);
  Tcl_Release(P->interp);
}

void UI::option(const LV2_Options_Option &option) {
}

LV2UI_Widget UI::widget() const {
  if (!P->widget)
    P->create_widget();
  return LV2UI_Widget(P->widget);
}

unsigned UI::width() {
  return pd_patch_info().root_canvas_size[0];
}

unsigned UI::height() {
  return pd_patch_info().root_canvas_size[1];
}

void UI::port_event(
    uint32_t port_index, uint32_t buffer_size, uint32_t format, const void *buffer) {
}

bool UI::needs_idle_callback() {
  return true;
}

bool UI::idle() {
  while (Tcl_DoOneEvent(TCL_DONT_WAIT)) {}
  return true;
}

//==============================================================================
void UI::Impl::create_widget() {
  bool success = false;

  const uintptr_t parent = reinterpret_cast<uintptr_t>(this->parent);

  Tcl_Interp *interp = Tcl_CreateInterp();
  if (!interp)
    throw std::runtime_error("error creating the Tcl interpreter\n");
  this->interp = interp;

  SCOPE_EXIT {
    if (!success) {
      Tcl_DeleteInterp(interp);
      this->interp = nullptr;
    }
  };

  Tcl_InitMemory(interp);

  Tcl_Preserve(interp);
  SCOPE_EXIT { if (!success) Tcl_Release(interp); };

  const std::string argv[] = {
    PROJECT_NAME,
    "-use", std::to_string(parent),
    "-geometry", std::to_string(UI::width()) + 'x' + std::to_string(UI::height())};
  Impl::tk_init(argv, sizeof(argv) / sizeof(argv[0]));

  Impl::tk_exec();

  // do some events, to make Tk create the wrapper window we need.
  while (Tcl_DoOneEvent(TCL_DONT_WAIT)) {}
  // Tk does not give us the window LV2 wants, go up and search for it.
  uintptr_t widget_id = find_widget(interp, parent);

  this->widget = reinterpret_cast<void *>(widget_id);
  success = true;
}

void UI::Impl::tk_init(const std::string *args, unsigned count) {
  Tcl_Interp *interp = this->interp;

  // pass arguments to the wish interpreter
  Tcl_SetVar2Ex(interp, "argc", nullptr, Tcl_NewIntObj(count), TCL_GLOBAL_ONLY);
  Tcl_SetVar(interp, "argv0", args[0].c_str(), TCL_GLOBAL_ONLY);
  Tcl_Obj *argv_obj = Tcl_NewListObj(0, nullptr);
  for (unsigned i = 0; i < count; ++i) {
    Tcl_Obj *arg_obj = Tcl_NewStringObj(args[i].data(), args[i].size());
    Tcl_ListObjAppendElement(interp, argv_obj, arg_obj);
  }
  Tcl_SetVar2Ex(interp, "argv", NULL, argv_obj, TCL_GLOBAL_ONLY);

  // initialize Tcl and Tk
  if (Tcl_Init(interp) == TCL_ERROR)
    throw std::runtime_error("error initializing Tcl\n");
  if (Tk_Init(interp) == TCL_ERROR)
    throw std::runtime_error("error initializing Tk\n");
  Tcl_StaticPackage(interp, "Tk", Tk_Init, Tk_SafeInit);
}

void UI::Impl::tk_exec() {
  Tcl_Interp *interp = this->interp;

  const PdPatchInfo &info = pd_patch_info();
  std::string script_path = info.lib_dir + "/tcl/pd-gui.tcl";
#warning TODO execute the GUI
  

  // run your Tcl script here
  std::string script =
      "button .hello -text {Hello, World!}\n"
      "pack .hello\n";
  int ret = Tcl_Eval(interp, script.c_str());

  if (ret != TCL_OK) {
    if (ret == TCL_ERROR)
      std::cerr << Tcl_GetStringResult(interp) << "\n";
    throw std::runtime_error("error executing the Tk program");
  }
}

uintptr_t UI::Impl::find_widget(Tcl_Interp *interp, uintptr_t parent) {
  Tk_Window main_window = Tk_MainWindow(interp);
  Tk_MakeWindowExist(main_window);
  Display *dpy = Tk_Display(main_window);
  uintptr_t window_id = Tk_WindowId(main_window);
  uintptr_t widget_id = window_id;
  for (uintptr_t p {}; widget_id &&
           (p = Impl::parent_window(dpy, widget_id)) != parent;)
    widget_id = p;
  if (!widget_id)
    widget_id = window_id;
  return widget_id;
}

uintptr_t UI::Impl::parent_window(Display *dpy, uintptr_t w) {
#if defined(_WIN32)
    return uintptr_t(GetParent(HWND(w)));
#elif defined(__unix__) && !defined(__APPLE__)
    Window r {}, p {};
    Window *c {};
    unsigned nc {};
    XQueryTree(dpy, w, &r, &p, &c, &nc);
    XFree(c);
    return p;
#else
# error Tk on Mac OS is not implemented
#endif
}
