#include "framework/description.h"
#include "framework/lv2all.h"
#include "utility/pd-patchinfo.h"

//==============================================================================
static EffectManifest create_effect_manifest() {
  EffectManifest m;
  m.uri = effect_uri;
  m.name = PROJECT_DISPLAY_NAME;

  // request features
  m.features.push_back(FeatureRequest{LV2_URID__map, RequiredFeature::Yes});
  m.features.push_back(FeatureRequest{LV2_URID__unmap, RequiredFeature::Yes});
  m.features.push_back(FeatureRequest{LV2_OPTIONS__options, RequiredFeature::No});
  m.features.push_back(FeatureRequest{LV2_BUF_SIZE__fixedBlockLength, RequiredFeature::No});

  // ports according to patch info
  const PdPatchInfo &info = pd_patch_info();
  for (unsigned i = 0, n = info.adc_count; i < n; ++i) {
    std::unique_ptr<AudioPort> p(new AudioPort);
    p->direction = PortDirection::Input;
    p->symbol = "input_" + std::to_string(i + 1);
    p->name = "Input " + std::to_string(i + 1);
    m.ports.emplace_back(std::move(p));
  }
  for (unsigned i = 0, n = info.dac_count; i < n; ++i) {
    std::unique_ptr<AudioPort> p(new AudioPort);
    p->direction = PortDirection::Output;
    p->symbol = "output_" + std::to_string(i + 1);
    p->name = "Output " + std::to_string(i + 1);
    m.ports.emplace_back(std::move(p));
  }
  if (info.has_midi_in) {
    std::unique_ptr<EventPort> p(new EventPort);
    p->direction = PortDirection::Input;
    p->symbol = "event_input";
    p->name = "Event input";
    p->buffer_type = LV2_ATOM__Sequence;
    p->supports.push_back(LV2_MIDI__MidiEvent);
    m.ports.emplace_back(std::move(p));
  }
  if (info.has_midi_out) {
    std::unique_ptr<EventPort> p(new EventPort);
    p->direction = PortDirection::Output;
    p->symbol = "event_output";
    p->name = "Event output";
    p->buffer_type = LV2_ATOM__Sequence;
    p->supports.push_back(LV2_MIDI__MidiEvent);
    m.ports.emplace_back(std::move(p));
  }

  return m;
}

//==============================================================================
static boost::optional<UIManifest> create_ui_manifest() {
  UIManifest m;
  m.uri = ui_uri;
  m.effect_uri = effect_uri;

  // [!!!IMPORTANT!!!] set UI class
  m.uiclass = LV2_UI__PlatformSpecificUI;

  // request features
  m.features.push_back(FeatureRequest{LV2_URID__map, RequiredFeature::Yes});
  m.features.push_back(FeatureRequest{LV2_URID__unmap, RequiredFeature::Yes});
  m.features.push_back(FeatureRequest{LV2_UI__resize, RequiredFeature::No});
  m.features.push_back(FeatureRequest{LV2_UI__parent, RequiredFeature::No});
  m.features.push_back(FeatureRequest{LV2_UI__idleInterface, RequiredFeature::Yes});

  // extension data
  m.extension_data.push_back(LV2_UI__idleInterface);

  return m;
}

//==============================================================================
const EffectManifest effect_manifest = create_effect_manifest();
const boost::optional<UIManifest> ui_manifest = create_ui_manifest();
