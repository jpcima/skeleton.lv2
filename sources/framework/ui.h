#pragma once
#include "lv2all.h"
#include <boost/utility/string_view.hpp>
#include <memory>
#include <cstdint>

class UI {
 public:
  UI(void *parent, LV2_URID_Map *map, LV2_URID_Unmap *unmap,
     const char *bundle_path);
  ~UI();

  void option(const LV2_Options_Option &o);

  LV2UI_Widget widget() const;
  static unsigned width();
  static unsigned height();

  void port_event(
      uint32_t port_index, uint32_t buffer_size, uint32_t format, const void *buffer);

  static bool needs_idle_callback();
  bool idle();

 private:
  struct Impl;
  const std::unique_ptr<Impl> P;
};
