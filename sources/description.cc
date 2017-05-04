#include "framework/description.h"
#include "framework/lv2all.h"

static EffectManifest create_effect_manifest() {
  EffectManifest m;
  m.uri = effect_uri;
  m.name = PROJECT_DISPLAY_NAME;

  // set effect categories (superclasses other than lv2:Plugin)
  m.categories.push_back(LV2_CORE__InstrumentPlugin);

  // request features
  m.features.push_back(FeatureRequest{LV2_URID__map, RequiredFeature::Yes});
  m.features.push_back(FeatureRequest{LV2_URID__unmap, RequiredFeature::Yes});

  // create audio ports
  for (unsigned i = 0; i < 2; ++i) {
    std::unique_ptr<AudioPort> p(new AudioPort);
    p->direction = PortDirection::Output;
    p->symbol = "audio_output_" + std::to_string(i + 1);
    p->name = "Audio output " + std::to_string(i + 1);
    m.ports.emplace_back(std::move(p));
  }

  // create event ports
  {
    std::unique_ptr<EventPort> p(new EventPort);
    p->direction = PortDirection::Input;
    p->symbol = "event_input";
    p->name = "Event input";
    p->buffer_type = LV2_ATOM__Sequence;
    p->supports.push_back(LV2_MIDI__MidiEvent);
    m.ports.emplace_back(std::move(p));
  }

  return m;
}

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

const EffectManifest effect_manifest = create_effect_manifest();
const boost::optional<UIManifest> ui_manifest = create_ui_manifest();
