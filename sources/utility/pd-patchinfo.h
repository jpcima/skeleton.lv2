#pragma once
#include <boost/property_tree/ptree.hpp>
#include <string>

struct PdPatchInfo {
  boost::property_tree::ptree ini;
  std::string patch_path {};
  std::string patch_base {};
  std::string patch_dir {};
  std::string lib_dir {};
  unsigned adc_count {};
  unsigned dac_count {};
  bool has_midi_in {};
  bool has_midi_out {};
  int root_canvas_pos[2] {};
  unsigned root_canvas_size[2] {};
  unsigned font_size {};
};

const PdPatchInfo &pd_patch_info();
