#include "framework/ui.h"
#include <QtWidgets/QWidget>

struct UI::Impl {
  static constexpr unsigned width = 600;
  static constexpr unsigned height = 400;
  QWidget *widget = nullptr;
  void create_widget();
};

//==============================================================================
UI::UI(void *parent)
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
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>

void UI::Impl::create_widget() {
  QWidget *editor = new QWidget;

  editor->setMinimumSize(Impl::width, Impl::height);
  editor->setMaximumSize(Impl::width, Impl::height);

  // create the GUI
  QVBoxLayout *vl = new QVBoxLayout;
  editor->setLayout(vl);
  QLabel *label = new QLabel("Hello, World!");
  label->setAlignment(Qt::AlignCenter|Qt::AlignHCenter);
  vl->addWidget(label);

  this->widget = editor;
}
