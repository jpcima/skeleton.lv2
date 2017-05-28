#include "framework/effect.h"
#include "framework/lv2all.h"
#include "utility/pd-patchinfo.h"
#include "utility/pd-common.h"
#include "utility/sys-signal.h"
#include "utility/scope-guard.h"
#include <lv2/lv2plug.in/ns/ext/atom/forge.h>
#include <z_libpd.h>
extern "C" {
#include <s_stuff.h>
}
#include <boost/utility/string_view.hpp>
#include <algorithm>
#include <mutex>
#include <iostream>
#include <cassert>
#include <unistd.h>

static bool libpd_initialized = false;

struct Effect::Impl {
  Impl();

  struct {
    LV2_URID midi_event;
  } urid;

  t_pdinstance *pd {};
  void *patch {};
  unsigned blocksize {};
  unsigned blockavail {};

  unsigned minblocksize {};
  unsigned maxblocksize {};

  std::unique_ptr<const float *[]> a_ins;
  std::unique_ptr<float *[]> a_outs;
  const LV2_Atom_Sequence *seq_in {};
  LV2_Atom_Sequence *seq_out {};
  LV2_Atom_Forge forge_out;

  LV2_URID_Map *map {};
  LV2_URID_Unmap *unmap {};

  static Impl *midihook_context;
  static void install_midi_hooks();
  void forge_midi(uint32_t ftime, const uint8_t *msg, uint32_t len);
  void midi_to_pd(const uint8_t *msg, uint32_t len);
  void noteoff_from_pd(int chan, int pitch);
  void noteon_from_pd(int chan, int pitch, int velocity);
  void controlchange_from_pd(int chan, int control, int value);
  void programchange_from_pd(int chan, int program);
  void pitchbend_from_pd(int chan, int bend);
  void aftertouch_from_pd(int chan, int value);
  void polyaftertouch_from_pd(int chan, int pitch, int value);
};

//==============================================================================
Effect::Effect(double rate, LV2_URID_Map *map, LV2_URID_Unmap *unmap)
    : P(new Impl) {
  bool success = false;

  if (!libpd_initialized)
    throw std::runtime_error("could not initialize puredata");

  P->urid.midi_event = map->map(map->handle, LV2_MIDI__MidiEvent);
  P->map = map;
  P->unmap = unmap;
  lv2_atom_forge_init(&P->forge_out, map);

  signal_blocker fpe_blocker(SIGFPE);
  fpe_blocker.activate();

  const PdPatchInfo &info = pd_patch_info();
  P->a_ins.reset(new const float *[info.adc_count]());
  P->a_outs.reset(new float *[info.dac_count]());

  P->pd = pdinstance_new();
  SCOPE_EXIT { if (!success) pdinstance_free(P->pd); };

  pd_setinstance(P->pd);
  P->patch = libpd_openfile(info.patch_base.c_str(), info.patch_dir.c_str());
  if (!P->patch)
    throw std::runtime_error("could not load the patch in puredata");
  SCOPE_EXIT { if (!success) libpd_closefile(P->patch); };

  if (libpd_init_audio(info.adc_count, info.dac_count, rate) != 0)
    throw std::runtime_error("could not initialize audio");

  P->blocksize = libpd_blocksize();
  pd_logs() << "[pd] block size: " << P->blocksize << "\n";

  success = true;
}

Effect::~Effect() {
  pd_setinstance(P->pd);
  libpd_closefile(P->patch);
  pdinstance_free(P->pd);
}

//==============================================================================
void Effect::option(const LV2_Options_Option &o) {
  LV2_URID_Map *map = P->map;

  if (0) {
    LV2_URID_Unmap *unmap = P->unmap;
    boost::string_view name = unmap->unmap(unmap->handle, o.key);
    std::cerr << "Option: " << name << "\n";
  }

  const uint32_t *value_uint = reinterpret_cast<const uint32_t *>(o.value);

  if (o.key == map->map(map->handle, LV2_BUF_SIZE__maxBlockLength))
    P->maxblocksize = *value_uint;
  else if (o.key == map->map(map->handle, LV2_BUF_SIZE__minBlockLength))
    P->minblocksize = *value_uint;
}

//==============================================================================
void Effect::connect_port(uint32_t port, void *data) {
  const PdPatchInfo &info = pd_patch_info();

  if (port < info.adc_count) {
    P->a_ins[port] = (const float *)data;
    return;
  }
  port -= info.adc_count;

  if (port < info.dac_count) {
    P->a_outs[port] = (float *)data;
    return;
  }
  port -= info.dac_count;

  if (port < info.has_midi_in) {
    P->seq_in = (const LV2_Atom_Sequence *)data;
    return;
  }
  port -= info.has_midi_in;

  if (port < info.has_midi_out) {
    P->seq_out = (LV2_Atom_Sequence *)data;
    return;
  }
  assert(false);
}

//==============================================================================
void Effect::activate() {
  signal_blocker fpe_blocker(SIGFPE);
  fpe_blocker.activate();

  Impl::midihook_context = nullptr;

  pd_setinstance(P->pd);

  // enable audio (pd dsp 1)
  libpd_start_message(1);
  libpd_add_float(1);
  libpd_finish_message("pd", "dsp");

  // run the patch once, in case of inits in the first run breaking RT
  const PdPatchInfo &info = pd_patch_info();
  const unsigned bs = P->blocksize;
  std::fill_n(STUFF->st_soundin, info.adc_count * bs, 0);
  sys_microsleep(0);
  sched_tick();

  P->blockavail = 0;
}

void Effect::deactivate() {
  signal_blocker fpe_blocker(SIGFPE);
  fpe_blocker.activate();

  Impl::midihook_context = nullptr;

  pd_setinstance(P->pd);

  // disable audio (pd dsp 0)
  libpd_start_message(1);
  libpd_add_float(0);
  libpd_finish_message("pd", "dsp");
}

//==============================================================================
void Effect::run(unsigned nframes) {
  const auto urid = P->urid;

  const PdPatchInfo &info = pd_patch_info();
  const unsigned adc_count = info.adc_count;
  const unsigned dac_count = info.dac_count;

  signal_blocker fpe_blocker(SIGFPE);
  fpe_blocker.activate();

  pd_setinstance(P->pd);

  float *soundin = STUFF->st_soundin;
  const float *soundout = STUFF->st_soundout;

  const unsigned bs = P->blocksize;
  unsigned ba = P->blockavail;

  const LV2_Atom_Sequence *seq_in = P->seq_in;
  LV2_Atom_Event *ev_in {};
  if (seq_in)
    ev_in = lv2_atom_sequence_begin(&seq_in->body);

  LV2_Atom_Sequence *seq_out = P->seq_out;
  LV2_Atom_Forge *forge_out {};
  LV2_Atom_Forge_Frame frame_out {};

  if (seq_out) {
    forge_out = &P->forge_out;
    uint8_t *buf = reinterpret_cast<uint8_t *>(seq_out);
    lv2_atom_forge_set_buffer(forge_out, buf, lv2_atom_total_size(&seq_out->atom));
    lv2_atom_forge_sequence_head(forge_out, &frame_out, 0);
  }

  Impl::midihook_context = P.get();

  auto do_midiin = [&](unsigned i) {
    if (!seq_in)
      return;
    while (!lv2_atom_sequence_is_end(&seq_in->body, seq_in->atom.size, ev_in)) {
      if (ev_in->body.type == urid.midi_event) {
        const uint8_t *msg = reinterpret_cast<const uint8_t *>(
            LV2_ATOM_CONTENTS_CONST(LV2_Atom_Event, ev_in));
        P->midi_to_pd(msg, ev_in->body.size);
      }
      ev_in = lv2_atom_sequence_next(ev_in);
    }
  };

  auto pd_cycle = [&](unsigned i) {
    do_midiin(i);
    std::fill_n(STUFF->st_soundout, info.dac_count * bs, 0);
    sys_microsleep(0);
    sched_tick();
    ba = bs;
  };

  // There is one pd buffer of input latency (by default 64 = 1.45ms@44.1kHz).
  // It ensures we always have enough samples to feed pd. However this latency
  // can be avoided when the frame count is fixed and multiple of the pd buffer.
  const bool nolatency =
      P->maxblocksize && P->maxblocksize == P->minblocksize &&
      (P->maxblocksize % bs) == 0;

  for (unsigned iframe = 0; iframe < nframes;) {
    if (nolatency) {
      //========================================================================
      for (unsigned i = 0; i < adc_count; ++i)
        std::copy_n(P->a_ins[i] + iframe, bs, soundin + i * bs);

      pd_cycle(iframe);

      for (unsigned i = 0; i < dac_count; ++i)
        std::copy_n(soundout + i * bs, bs, P->a_outs[i] + iframe);

      iframe += bs;
    } else {
      //========================================================================
      if (ba == 0)
        pd_cycle(iframe);

      const unsigned ncopy = std::min(ba, nframes - iframe);

      for (unsigned i = 0; i < adc_count; ++i)
        std::copy_n(P->a_ins[i] + iframe, ncopy, soundin + i * bs + bs - ba);
      for (unsigned i = 0; i < dac_count; ++i)
        std::copy_n(soundout + i * bs + bs - ba, ncopy, P->a_outs[i] + iframe);

      iframe += ncopy;
      ba -= ncopy;
    }
  }

  if (seq_out) {
    lv2_atom_forge_pop(forge_out, &frame_out);
  }

  Impl::midihook_context = nullptr;

  P->blockavail = ba;
}

//==============================================================================
Effect::Impl *Effect::Impl::midihook_context = nullptr;

Effect::Impl::Impl() {
  Impl::install_midi_hooks();
}

void Effect::Impl::install_midi_hooks() {
  static volatile bool installed = false;
  static std::mutex mutex;

  if (installed)
    return;

  std::lock_guard<std::mutex> lock(mutex);
  if (installed)
    return;

  libpd_set_noteonhook([](int c, int p, int v) {
      if (midihook_context) {
        if (v) midihook_context->noteon_from_pd(c, p, v);
        else midihook_context->noteoff_from_pd(c, p);
      }
    });
  libpd_set_controlchangehook([](int c, int n, int v) {
    if (midihook_context) midihook_context->controlchange_from_pd(c, n, v); });
  libpd_set_programchangehook([](int c, int n) {
    if (midihook_context) midihook_context->programchange_from_pd(c, n); });
  libpd_set_pitchbendhook([](int c, int v) {
    if (midihook_context) midihook_context->pitchbend_from_pd(c, v); });
  libpd_set_aftertouchhook([](int c, int v) {
    if (midihook_context) midihook_context->aftertouch_from_pd(c, v); });
  libpd_set_polyaftertouchhook([](int c, int n, int v) {
    if (midihook_context) midihook_context->polyaftertouch_from_pd(c, n, v); });

  installed = true;
}

void Effect::Impl::forge_midi(
    uint32_t ftime, const uint8_t *msg, uint32_t len) {
  LV2_Atom_Forge *forge = &this->forge_out;
  if (!forge) return;

  LV2_Atom_Event event;
  event.time.frames = ftime;
  event.body.size = len;
  event.body.type = this->urid.midi_event;
  uint32_t fullsize = sizeof(event) + len;  // header + content

  lv2_atom_forge_raw(forge, &event, sizeof(event));
  lv2_atom_forge_raw(forge, msg, len);
  lv2_atom_forge_pad(forge, fullsize);
}

void Effect::Impl::noteoff_from_pd(int chan, int pitch) {
  uint8_t buf[3];
  buf[0] = LV2_MIDI_MSG_NOTE_OFF | chan;
  buf[1] = pitch;
  buf[2] = 0;
  this->forge_midi(0, buf, sizeof(buf));
}

void Effect::Impl::noteon_from_pd(int chan, int pitch, int velocity) {
  uint8_t buf[3];
  buf[0] = LV2_MIDI_MSG_NOTE_ON | chan;
  buf[1] = pitch;
  buf[2] = velocity;
  this->forge_midi(0, buf, sizeof(buf));
}

void Effect::Impl::controlchange_from_pd(int chan, int control, int value) {
  uint8_t buf[3];
  buf[0] = LV2_MIDI_MSG_CONTROLLER | chan;
  buf[1] = control;
  buf[2] = value;
  this->forge_midi(0, buf, sizeof(buf));
}

void Effect::Impl::programchange_from_pd(int chan, int program) {
  uint8_t buf[2];
  buf[0] = LV2_MIDI_MSG_CONTROLLER | chan;
  buf[1] = program;
  this->forge_midi(0, buf, sizeof(buf));
}

void Effect::Impl::pitchbend_from_pd(int chan, int bend) {
  uint8_t buf[3];
  buf[0] = LV2_MIDI_MSG_BENDER | chan;
  buf[1] = (bend + 8192) & 127;
  buf[2] = ((bend + 8192) / 128) & 127;
  this->forge_midi(0, buf, sizeof(buf));
}

void Effect::Impl::aftertouch_from_pd(int chan, int value) {
  uint8_t buf[2];
  buf[0] = LV2_MIDI_MSG_CHANNEL_PRESSURE | chan;
  buf[1] = value;
  this->forge_midi(0, buf, sizeof(buf));
}

void Effect::Impl::polyaftertouch_from_pd(int chan, int pitch, int value) {
  uint8_t buf[3];
  buf[0] = LV2_MIDI_MSG_NOTE_PRESSURE | chan;
  buf[1] = pitch;
  buf[2] = value;
  this->forge_midi(0, buf, sizeof(buf));
}

void Effect::Impl::midi_to_pd(const uint8_t *msg, uint32_t len) {
  if (lv2_midi_is_voice_message(msg)) {
    unsigned stat = msg[0] & 0x0f0;
    unsigned chan = msg[0] & 0x0f;
    unsigned d1 = (len > 1) ? msg[1] : 0;
    unsigned d2 = (len > 2) ? msg[2] : 0;
    switch (stat) {
      case LV2_MIDI_MSG_NOTE_OFF:
        inmidi_noteon(0, chan, d1, 0); break;
      case LV2_MIDI_MSG_NOTE_ON:
        inmidi_noteon(0, chan, d1, d2); break;
      case LV2_MIDI_MSG_CONTROLLER:
        inmidi_controlchange(0, chan, d1, d2); break;
      case LV2_MIDI_MSG_PGM_CHANGE:
        inmidi_programchange(0, chan, d1); break;
      case LV2_MIDI_MSG_BENDER:
        inmidi_pitchbend(0, chan, (d1 << 7) | d2); break;
      case LV2_MIDI_MSG_NOTE_PRESSURE:
        inmidi_polyaftertouch(0, chan, d1, d2); break;
      case LV2_MIDI_MSG_CHANNEL_PRESSURE:
        inmidi_aftertouch(0, chan, d1); break;
      default:
        break;
    }
  }
  // TODO: sysex messages?
}

//==============================================================================
[[gnu::constructor]] static void init_plugin() {
  if (pd_verbose) {
    const char msg[] = "[pd] initialize pd\n";
    write(2, msg, sizeof(msg) - 1);
  }

  signal_blocker fpe_blocker(SIGFPE);
  fpe_blocker.activate();

  libpd_initialized = libpd_init() == 0;
}

[[gnu::destructor]] static void fini_plugin() {
  if (pd_verbose) {
    const char msg[] = "[pd] finalize pd\n";
    write(2, msg, sizeof(msg) - 1);
  }

  signal_blocker fpe_blocker(SIGFPE);
  fpe_blocker.activate();

  libpd_term();
}
