#include "framework/effect.h"
#include "framework/lv2all.h"
#include "utility/pd-patchinfo.h"
#include "utility/pd-common.h"
#include "utility/sys-signal.h"
#include "utility/scope-guard.h"
#include <z_libpd.h>
extern "C" {
#include <s_stuff.h>
}
#include <boost/utility/string_view.hpp>
#include <algorithm>
#include <iostream>
#include <cassert>
#include <unistd.h>

static bool libpd_initialized = false;

struct Effect::Impl {
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
  const LV2_Atom_Sequence *ev_in {};
  LV2_Atom_Sequence *ev_out {};

  LV2_URID_Map *map {};
  LV2_URID_Unmap *unmap {};
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
  signal_blocker fpe_blocker(SIGFPE);
  fpe_blocker.activate();

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

  auto pd_cycle = [&]() {
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

#warning TODO midi messages

  for (unsigned iframe = 0; iframe < nframes;) {
    if (nolatency) {
      //========================================================================
      for (unsigned i = 0; i < adc_count; ++i)
        std::copy_n(P->a_ins[i] + iframe, bs, soundin + i * bs);

      pd_cycle();

      for (unsigned i = 0; i < dac_count; ++i)
        std::copy_n(soundout + i * bs, bs, P->a_outs[i] + iframe);

      iframe += bs;
    } else {
      //========================================================================
      if (ba == 0)
        pd_cycle();

      const unsigned ncopy = std::min(ba, nframes - iframe);

      for (unsigned i = 0; i < adc_count; ++i)
        std::copy_n(P->a_ins[i] + iframe, ncopy, soundin + i * bs + bs - ba);
      for (unsigned i = 0; i < dac_count; ++i)
        std::copy_n(soundout + i * bs + bs - ba, ncopy, P->a_outs[i] + iframe);

      iframe += ncopy;
      ba -= ncopy;
    }
  }

  P->blockavail = ba;
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
