#include "pd-patchinfo.h"
#include "pd-miniparser.h"
#include "pd-common.h"
#include <boost/property_tree/ini_parser.hpp>
#include <boost/filesystem.hpp>
#include <boost/dll.hpp>
#include <atomic>
#include <memory>
#include <mutex>
namespace ptree = boost::property_tree;
namespace fs = boost::filesystem;
namespace dll = boost::dll;

static std::atomic<PdPatchInfo *> patch_info_ptr;
static std::unique_ptr<PdPatchInfo> patch_info_raii;
static std::mutex patch_info_mutex;

const PdPatchInfo &pd_patch_info() {
  if (PdPatchInfo *ptr = patch_info_ptr)  // RT-safe if done once already
    return *ptr;

  std::lock_guard<std::mutex> lock(patch_info_mutex);
  if (PdPatchInfo *ptr = patch_info_ptr)
    return *ptr;

  PdPatchInfo *info = new PdPatchInfo;
  patch_info_raii.reset(info);

  // identify the plugin directory
  fs::path plugin_dir = dll::symbol_location(pd_patch_info).parent_path();
  fs::path pd_ini_path = plugin_dir / "pd.ini";
  pd_logs() << "[pd] config: " << pd_ini_path << "\n";

  // load pd.ini and extract info from patch
  ptree::ptree &pd_ini = info->ini;
  ptree::ini_parser::read_ini(pd_ini_path.native(), pd_ini);

  fs::path pd_patch_path = plugin_dir / pd_ini.get<std::string>("puredata.patch-file");
  pd_logs() << "[pd] patch: " << pd_patch_path << "\n";
  info->patch_path = pd_patch_path.string();
  info->patch_base = pd_patch_path.filename().string();
  info->patch_dir = pd_patch_path.parent_path().string();

  fs::path pd_lib_dir = plugin_dir / pd_ini.get<std::string>("puredata.lib-dir");
  pd_logs() << "[pd] lib dir: " << pd_lib_dir << "\n";
  info->lib_dir = pd_lib_dir.string();

  // load the patch
  const std::vector<std::string> records = pd_file_read_records(info->patch_path);
  pd_logs() << "[pd] patch loaded, " << records.size() << " records\n";

  if (!pd_patch_getinfo(records,
                        &info->adc_count, &info->dac_count,
                        &info->has_midi_in, &info->has_midi_out,
                        info->root_canvas_pos, info->root_canvas_size,
                        &info->font_size))
    throw std::runtime_error("error getting patch info");
  pd_logs() <<
      "[pd] ADC count: " << info->adc_count << "\n"
      "[pd] DAC count: " << info->dac_count << "\n"
      "[pd] MIDI input: " << (info->has_midi_in ? "yes" : "no") << "\n"
      "[pd] MIDI output: " << (info->has_midi_out ? "yes" : "no") << "\n"
      "[pd] canvas position: " << info->root_canvas_pos[0] << ' '
                         << info->root_canvas_pos[1] << "\n"
      "[pd] canvas size: " << info->root_canvas_size[0] << ' '
                         << info->root_canvas_size[1] << "\n"
      "[pd] font size: " << info->font_size << "\n";

  patch_info_ptr = info;
  return *info;
}
