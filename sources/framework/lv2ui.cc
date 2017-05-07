#include "ui.h"
#include "description.h"
#include "lv2all.h"
#include <boost/utility/string_view.hpp>
#include <iostream>
#include <memory>
#include <cassert>

static LV2UI_Handle instantiate(const LV2UI_Descriptor *descriptor,
                                const char *plugin_uri,
                                const char *bundle_path,
                                LV2UI_Write_Function write_function,
                                LV2UI_Controller controller,
                                LV2UI_Widget *widget,
                                const LV2_Feature *const *features) {
  LV2_URID_Map *map {};
  LV2_URID_Unmap *unmap {};
  LV2UI_Resize *resize {};
  void *parent {};

  for (const LV2_Feature *const *p = features, *f; (f = *p); ++p) {
    boost::string_view uri = f->URI;
    if (uri == LV2_URID__map)
      map = reinterpret_cast<LV2_URID_Map *>(f->data);
    else if (uri == LV2_URID__unmap)
      unmap = reinterpret_cast<LV2_URID_Unmap *>(f->data);
    else if (uri == LV2_UI__resize)
      resize = reinterpret_cast<LV2UI_Resize *>(f->data);
    else if (uri == LV2_UI__parent)
      parent = f->data;
  }

  assert(map);
  assert(unmap);

  std::unique_ptr<UI> ui;
  try {
    ui.reset(new UI(parent, map, unmap));
    *widget = ui->widget();
    if (resize)
      resize->ui_resize(resize->handle, UI::width(), UI::height());
  } catch (std::exception &ex) {
    std::cerr << "error instanciating: " << ex.what() << "\n";
    *widget = nullptr;
    return nullptr;
  }
  return ui.release();
}

static void cleanup(LV2UI_Handle handle) {
  UI *ui = reinterpret_cast<UI *>(handle);
  delete ui;
}

static void port_event(LV2UI_Handle handle,
                       uint32_t port_index, uint32_t buffer_size,
                       uint32_t format, const void *buffer) {
  UI *ui = reinterpret_cast<UI *>(handle);
  ui->port_event(port_index, buffer_size, format, buffer);
}

static int idle(LV2UI_Handle handle) {
  UI *ui = reinterpret_cast<UI *>(handle);
  return ui->idle();
}

static const void *extension_data(const char *uri_) {
  boost::string_view uri = uri_;
  if (uri == LV2_UI__idleInterface) {
    if (UI::needs_idle_callback()) {
      static const LV2UI_Idle_Interface intf = { &idle };
      return &intf;
    }
  }
  return nullptr;
}

static const LV2UI_Descriptor descriptor = {
  ui_uri,
  instantiate,
  cleanup,
  port_event,
  extension_data,
};

LV2_SYMBOL_EXPORT
const LV2UI_Descriptor *lv2ui_descriptor(uint32_t index) {
  switch (index) {
    case 0: return &descriptor;
    default: return nullptr;
  }
}
