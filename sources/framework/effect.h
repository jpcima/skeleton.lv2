#pragma once
#include "lv2all.h"
#include <memory>
#include <cstdint>

class Effect {
 public:
  Effect(double rate, LV2_URID_Map *map, LV2_URID_Unmap *unmap);
  ~Effect();

  //============================================================================
  void connect_port(uint32_t port, void *data);

  //============================================================================
  void activate();
  void deactivate();

  //============================================================================
  void run(unsigned nframes);

 private:
  struct Impl;
  const std::unique_ptr<Impl> P;
};
