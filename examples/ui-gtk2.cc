#include "framework/ui.h"
#include <gtk/gtk.h>

struct UI::Impl {
  static constexpr unsigned width = 600;
  static constexpr unsigned height = 400;
  GtkWidget *widget = nullptr;
  void create_widget();
};

//==============================================================================
UI::UI(void *parent, LV2_URID_Map *map, LV2_URID_Unmap *unmap,
       const char *bundle_path)
    : P(new Impl) {
}

UI::~UI() {
}

void UI::option(const LV2_Options_Option &o) {
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
  return false;
}

bool UI::idle() {
  return false;
}

//==============================================================================
void UI::Impl::create_widget() {
  GtkWidget *editor = gtk_vbox_new(false, 0);
  gtk_widget_set_size_request(editor, Impl::width, Impl::height);
  GtkWidget *label = gtk_label_new("Hello, World!");
  gtk_box_pack_start(GTK_BOX(editor), label, true, true, 0);
  this->widget = editor;
}
