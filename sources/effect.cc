#include "framework/effect.h"
#include "framework/lv2all.h"
#include <cassert>

struct Effect::Impl {
  float *port_left = nullptr;
  float *port_right = nullptr;
  LV2_Atom_Sequence *port_events = nullptr;
};

//==============================================================================
Effect::Effect(double rate)
    : P(new Impl) {
}

Effect::~Effect() {
}

//==============================================================================
void Effect::connect_port(uint32_t port, void *data) {
  switch (port) {
    case 0: P->port_left = (float *)data; break;
    case 1: P->port_right = (float *)data; break;
    case 2: P->port_events = (LV2_Atom_Sequence *)data; break;
    default: assert(false);
  }
}

//==============================================================================
void Effect::activate() {
}

void Effect::deactivate() {
}

//==============================================================================
void Effect::run(unsigned nframes) {
  // TODO put audio code here
  std::fill_n(P->port_left, nframes, 0);
  std::fill_n(P->port_right, nframes, 0);
}
