#include "framework/description.h"
#include "framework/lv2all.h"
#include "utility/pd-miniparser.h"
#include "utility/pd-patchinfo.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/filesystem.hpp>
#include <boost/dll.hpp>
#include <iostream>
namespace ptree = boost::property_tree;
namespace fs = boost::filesystem;
namespace dll = boost::dll;

PdPatchInfo pd_patch_info;
///

static EffectManifest create_effect_manifest() {
  EffectManifest m;
  m.uri = effect_uri;
  m.name = PROJECT_DISPLAY_NAME;

  PdPatchInfo &info = ::pd_patch_info;

  // identify the plugin directory
  fs::path plugin_dir = dll::symbol_location(create_effect_manifest).parent_path();
  fs::path pd_ini_path = plugin_dir / "pd.ini";
  std::cerr << "[pd] config: " << pd_ini_path << "\n";

  // load pd.ini and extract info from patch
  ptree::ptree pd_ini;
  ptree::ini_parser::read_ini(pd_ini_path.native(), pd_ini);

  fs::path pd_patch_path = plugin_dir / pd_ini.get<std::string>("puredata.patch-file");
  std::cerr << "[pd] patch: " << pd_patch_path << "\n";
  info.patch_path = pd_patch_path.string();
  info.patch_base = pd_patch_path.filename().string();
  info.patch_dir = pd_patch_path.parent_path().string();

  // load the patch
  const std::vector<std::string> records = pd_file_read_records(info.patch_path);
  std::cerr << "[pd] patch loaded, " << records.size() << " records\n";

  pd_patch_getinfo(
      records,
      &info.adc_count, &info.dac_count,
      &info.has_midi_in, &info.has_midi_out);
  std::cerr <<
      "[pd] ADC count: " << info.adc_count << "\n"
      "[pd] DAC count: " << info.dac_count << "\n"
      "[pd] MIDI input: " << (info.has_midi_in ? "yes" : "no") << "\n"
      "[pd] MIDI output: " << (info.has_midi_out ? "yes" : "no") << "\n";

  // request features
  m.features.push_back(FeatureRequest{LV2_URID__map, RequiredFeature::Yes});
  m.features.push_back(FeatureRequest{LV2_URID__unmap, RequiredFeature::Yes});
  m.features.push_back(FeatureRequest{LV2_OPTIONS__options, RequiredFeature::No});
  m.features.push_back(FeatureRequest{LV2_BUF_SIZE__fixedBlockLength, RequiredFeature::No});

  // puredata ports
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
