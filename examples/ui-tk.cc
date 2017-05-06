#include "framework/ui.h"
#include "meta/project.h"
#include <tcl.h>
#include <tk.h>
#include <boost/lexical_cast.hpp>
#include <boost/scope_exit.hpp>
#include <sstream>
#include <iostream>

struct UI::Impl {
  static constexpr unsigned width = 600;
  static constexpr unsigned height = 400;
  Tcl_Interp *interp = nullptr;
  void *widget = nullptr;
  void *parent = nullptr;
  void create_widget();
  void tk_init(const std::string *args, unsigned count);
  void tk_exec();
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

  BOOST_SCOPE_EXIT(&success, this_, interp) {
    if (!success) { Tcl_DeleteInterp(interp); this_->interp = nullptr; }
  } BOOST_SCOPE_EXIT_END;

  Tcl_InitMemory(interp);

  Tcl_Preserve(interp);

  BOOST_SCOPE_EXIT(&success, interp) {
    if (!success) Tcl_Release(interp);
  } BOOST_SCOPE_EXIT_END;

  const std::string argv[] = {
    PROJECT_NAME,
    "-use", std::to_string(parent),
    "-geometry", std::to_string(Impl::width) + 'x' + std::to_string(Impl::height)};
  Impl::tk_init(argv, sizeof(argv) / sizeof(argv[0]));

  if (Tcl_Eval(interp, "winfo id .") != TCL_OK)
    throw std::runtime_error("error identifying the Tk root window");
  uintptr_t widget {};
  std::istringstream(Tcl_GetStringResult(interp)) >> widget;

  Impl::tk_exec();

  this->widget = reinterpret_cast<void *>(widget);
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
