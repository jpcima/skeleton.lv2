#pragma once
#include <string>

struct PdPatchInfo {
  std::string patch_path {};
  std::string patch_base {};
  std::string patch_dir {};
  unsigned adc_count {};
  unsigned dac_count {};
  bool has_midi_in {};
  bool has_midi_out {};
};
