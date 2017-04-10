#pragma once
#include "../meta/project.h"
#include <enum.h>
#include <memory>
#include <string>
#include <vector>

static constexpr char effect_uri[] = PROJECT_URI "#fx";
static constexpr char effect_binary_file[] = PROJECT_NAME ".fx";

static constexpr char ui_uri[] = PROJECT_URI "#ui";
static constexpr char ui_binary_file[] = PROJECT_NAME ".ui";

struct EffectManifest;
struct UIManifest;
extern const EffectManifest effect_manifest;
extern const UIManifest ui_manifest;

//==============================================================================
BETTER_ENUM(
    PortDirection, int,
    Input,
    Output);

BETTER_ENUM(
    PortKind, int,
    Audio,
    Control,
    Event);

BETTER_ENUM(
    RequiredFeature, bool,
    No,
    Yes);

struct FeatureRequest {
  std::string uri;
  RequiredFeature required = RequiredFeature::Yes;
};

struct Port {
  PortDirection direction = PortDirection::Input;
  std::string symbol;
  std::string name;
  std::string designation;
  virtual ~Port() {}
  virtual PortKind kind() const = 0;
};

struct AudioPort : Port {
  PortKind kind() const override { return PortKind::Audio; }
};

struct ControlPort : Port {
  float default_value {};
  float minimum_value {};
  float maximum_value {};
  PortKind kind() const override { return PortKind::Control; }
};

struct EventPort : Port {
  std::string buffer_type;
  std::string supports;
  PortKind kind() const override { return PortKind::Event; }
};

struct EffectManifest {
  std::string uri;
  std::string name;
  std::vector<std::string> categories;
  std::vector<FeatureRequest> features;
  std::vector<std::string> extension_data;
  std::vector<std::unique_ptr<Port>> ports;
};

struct UIManifest {
  std::string uri;
  std::string uiclass;
  std::vector<FeatureRequest> features;
  std::vector<std::string> extension_data;
};
