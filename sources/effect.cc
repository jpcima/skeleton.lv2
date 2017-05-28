#include "framework/effect.h"
#include "framework/lv2all.h"
#include "utility/pd-patchinfo.h"
#include "utility/sys-signal.h"
#include "utility/scope-guard.h"
#include <z_libpd.h>
extern "C" {
#include <s_stuff.h>
}
#include <iostream>
#include <cstring>
#include <cassert>
#include <unistd.h>

extern PdPatchInfo pd_patch_info;
static bool libpd_initialized = false;

struct Effect::Impl {
  struct {
    LV2_URID midi_event;
  } urid;

  t_pdinstance *pd {};
  void *patch {};
  unsigned blocksize {};
  unsigned blockavail {};

  std::unique_ptr<const float *[]> a_ins;
  std::unique_ptr<float *[]> a_outs;
  const LV2_Atom_Sequence *ev_in {};
  LV2_Atom_Sequence *ev_out {};

  signal_blocker fpe_blocker{SIGFPE};
};

//==============================================================================
Effect::Effect(double rate, LV2_URID_Map *map, LV2_URID_Unmap *unmap)
    : P(new Impl) {
  bool success = false;

  if (!libpd_initialized)
    throw std::runtime_error("could not initialize puredata");

  P->urid.midi_event = map->map(map->handle, LV2_MIDI__MidiEvent);

  signal_blocker fpe_blocker(SIGFPE);
  fpe_blocker.activate();

  const PdPatchInfo &info = ::pd_patch_info;
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
  std::cerr << "[pd] block size: " << P->blocksize << "\n";

  success = true;
}

Effect::~Effect() {
  pd_setinstance(P->pd);
  libpd_closefile(P->patch);
  pdinstance_free(P->pd);
}

//==============================================================================
void Effect::option(const LV2_Options_Option &o) {
}

//==============================================================================
void Effect::connect_port(uint32_t port, void *data) {
  const PdPatchInfo &info = ::pd_patch_info;

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
    P->ev_in = (const LV2_Atom_Sequence *)data;
    return;
  }
  port -= info.has_midi_in;

  if (port < info.has_midi_out) {
    P->ev_out = (LV2_Atom_Sequence *)data;
    return;
  }
  assert(false);
}

//==============================================================================
void Effect::activate() {
  P->fpe_blocker.activate();
  SCOPE_EXIT { P->fpe_blocker.deactivate(); };

  pd_setinstance(P->pd);

  // enable audio (pd dsp 1)
  libpd_start_message(1);
  libpd_add_float(1);
  libpd_finish_message("pd", "dsp");

  // run the patch once, in case of inits in the first run breaking RT
  const PdPatchInfo &info = ::pd_patch_info;
  const unsigned bs = P->blocksize;
  std::memset(STUFF->st_soundin, 0, info.adc_count * bs * sizeof(float));
  sys_microsleep(0);
  sched_tick();

  P->blockavail = 0;
}

void Effect::deactivate() {
  pd_setinstance(P->pd);

  // disable audio (pd dsp 0)
  libpd_start_message(1);
  libpd_add_float(0);
  libpd_finish_message("pd", "dsp");
}

//==============================================================================
void Effect::run(unsigned nframes) {
  const auto urid = P->urid;

  const PdPatchInfo &info = ::pd_patch_info;
  const unsigned adc_count = info.adc_count;
  const unsigned dac_count = info.dac_count;

  P->fpe_blocker.activate();
  SCOPE_EXIT { P->fpe_blocker.deactivate(); };

  pd_setinstance(P->pd);

  const unsigned bs = P->blocksize;
  unsigned ba = P->blockavail;

#warning TODO midi messages

  for (unsigned iframe = 0; iframe < nframes;) {
    // There is one pd buffer of input latency (by default 64 = 1.45ms@44.1kHz).
    // It ensures we always have enough samples to feed pd. This latency could
    // be avoided when the frame count is fixed and multiple of the pd buffer.
    if (ba == 0) {
      std::memset(STUFF->st_soundout, 0, info.dac_count * bs * sizeof(float));
      sys_microsleep(0);
      sched_tick();
      ba = bs;
    }

    float *soundin = STUFF->st_soundin;
    const float *soundout = STUFF->st_soundout;

    const unsigned ncopy = std::min(ba, nframes - iframe);

    for (unsigned i = 0; i < adc_count; ++i) {
      float *dst = soundin + i * bs + bs - ba;
      const float *src = P->a_ins[i] + iframe;
      std::memcpy(dst, src, ncopy * sizeof(float));
    }

    for (unsigned i = 0; i < dac_count; ++i) {
      float *dst = P->a_outs[i] + iframe;
      const float *src = soundout + i * bs + bs - ba;
      std::memcpy(dst, src, ncopy * sizeof(float));
    }

    iframe += ncopy;
    ba -= ncopy;
  }

  P->blockavail = ba;
}

//==============================================================================
[[gnu::constructor]] static void init_plugin() {
  const char msg[] = "[pd] initialize pd\n";
  write(2, msg, sizeof(msg) - 1);

  signal_blocker fpe_blocker(SIGFPE);
  fpe_blocker.activate();

  libpd_initialized = libpd_init() == 0;
}

[[gnu::destructor]] static void fini_plugin() {
  const char msg[] = "[pd] finalize pd\n";
  write(2, msg, sizeof(msg) - 1);

  signal_blocker fpe_blocker(SIGFPE);
  fpe_blocker.activate();

  libpd_term();
}
