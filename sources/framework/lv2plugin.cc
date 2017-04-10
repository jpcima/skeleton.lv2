#include "effect.h"
#include "description.h"
#include "lv2all.h"
#include <iostream>
#include <memory>
#include <stdexcept>

static LV2_Handle instantiate(
    const LV2_Descriptor *descriptor,
    double rate,
    const char *bundle_path,
    const LV2_Feature *const *features) {
  std::unique_ptr<Effect> fx;
  try {
    fx.reset(new Effect(rate));
  } catch (std::exception &ex) {
    std::cerr << "error instanciating: " << ex.what() << "\n";
    return nullptr;
  }
  return fx.release();
}

static void connect_port(LV2_Handle instance,
             uint32_t port,
             void *data) {
  Effect *fx = reinterpret_cast<Effect *>(instance);
  fx->connect_port(port, data);
}

static void activate(LV2_Handle instance) {
  Effect *fx = reinterpret_cast<Effect *>(instance);
  fx->activate();
}

static void run(LV2_Handle instance, uint32_t nframes) {
  Effect *fx = reinterpret_cast<Effect *>(instance);
  fx->run(nframes);
}

static void deactivate(LV2_Handle instance) {
  Effect *fx = reinterpret_cast<Effect *>(instance);
  fx->deactivate();
}

static void cleanup(LV2_Handle instance) {
  Effect *fx = reinterpret_cast<Effect *>(instance);
  delete fx;
}

static const void *extension_data(const char* uri) {
  return nullptr;
}

static const LV2_Descriptor descriptor = {
  effect_uri,
  instantiate,
  connect_port,
  activate,
  run,
  deactivate,
  cleanup,
  extension_data,
};

LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index) {
  switch (index) {
    case 0:  return &descriptor;
    default: return nullptr;
  }
}
