#include "framework/description.h"
#include "framework/lv2all.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/filesystem.hpp>
#include <boost/dll.hpp>
#include <iostream>
namespace ptree = boost::property_tree;
namespace fs = boost::filesystem;
namespace dll = boost::dll;

static EffectManifest create_effect_manifest() {
  EffectManifest m;
  m.uri = effect_uri;
  m.name = PROJECT_DISPLAY_NAME;

  // identify the plugin directory
  fs::path plugin_dir = dll::symbol_location(create_effect_manifest).parent_path();
  fs::path pd_ini_path = plugin_dir / "pd.ini";
  std::cerr << "Puredata config: " << pd_ini_path << "\n";

  // load pd.ini and extract info from patch
  ptree::ptree pd_ini;
  ptree::ini_parser::read_ini(pd_ini_path.native(), pd_ini);

  fs::path pd_patch_path = plugin_dir / pd_ini.get<std::string>("puredata.patch-file");
  std::cerr << "Puredata patch: " << pd_patch_path << "\n";

  
  

  // request features
  m.features.push_back(FeatureRequest{LV2_URID__map, RequiredFeature::Yes});
  m.features.push_back(FeatureRequest{LV2_URID__unmap, RequiredFeature::Yes});

  // TODO puredata ports
  

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
