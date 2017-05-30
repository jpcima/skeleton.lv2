#include "framework/effect.h"
#include "framework/lv2all.h"
#include <cassert>

struct Effect::Impl {
  float *port_left = nullptr;
  float *port_right = nullptr;
  LV2_Atom_Sequence *port_events = nullptr;
  unsigned in_midi_channel = 0;
  struct {
    LV2_URID midi_event;
  } urid;
};

//==============================================================================
Effect::Effect(double rate, LV2_URID_Map *map, LV2_URID_Unmap *unmap,
               const char *bundle_path)
    : P(new Impl) {
  P->urid.midi_event = map->map(map->handle, LV2_MIDI__MidiEvent);
}

Effect::~Effect() {
}

//==============================================================================
void Effect::option(const LV2_Options_Option &o) {
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
  const auto urid = P->urid;

  // TODO put midi code here
  LV2_ATOM_SEQUENCE_FOREACH(P->port_events, event) {
    if (event->body.type == urid.midi_event) {
      const uint8_t *msg = (uint8_t *)LV2_ATOM_CONTENTS(LV2_Atom_Event, event);
      uint32_t msglen = event->body.size;
      if (lv2_midi_is_system_message(msg) ||
          (lv2_midi_is_voice_message(msg) &&
           (msg[0] & 0xf) == P->in_midi_channel)) {
        // handle the midi message
      }
    }
  }

  // TODO put audio code here
  std::fill_n(P->port_left, nframes, 0);
  std::fill_n(P->port_right, nframes, 0);
}
