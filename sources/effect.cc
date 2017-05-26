#include "framework/effect.h"
#include "framework/lv2all.h"
#include <cassert>

struct Effect::Impl {
  struct {
    LV2_URID midi_event;
  } urid;
};

//==============================================================================
Effect::Effect(double rate, LV2_URID_Map *map, LV2_URID_Unmap *unmap)
    : P(new Impl) {
  P->urid.midi_event = map->map(map->handle, LV2_MIDI__MidiEvent);
}

Effect::~Effect() {
}

//==============================================================================
void Effect::connect_port(uint32_t port, void *data) {
  switch (port) {
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
  const auto urid = P->urid;

  // TODO implement me
}
