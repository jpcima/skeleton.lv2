#pragma once
#include <boost/utility/string_view.hpp>
#include <string>
#include <vector>
#include <iosfwd>
#include <stdexcept>

std::vector<std::string> pd_read_records(std::istream &in);
std::vector<std::string> pd_file_read_records(boost::string_view path);

bool pd_record_parse_obj(
    const std::string &record,
    int *x, int *y, std::string *exp);
bool pd_record_parse_root_canvas(
    const std::string &record,
    int pos[2], unsigned size[2], unsigned *fs);

boost::string_view pd_exp_cmd(const std::string &exp);
std::vector<std::string> pd_exp_tokens(const std::string &exp);
bool pd_exp_parse_adc(const std::string &exp, std::vector<int> &channels);
bool pd_exp_parse_dac(const std::string &exp, std::vector<int> &channels);

bool pd_patch_getinfo(
    const std::vector<std::string> &records,
    unsigned *adc_count, unsigned *dac_count,
    bool *has_midi_in, bool *has_midi_out,
    int root_canvas_pos[2], unsigned root_canvas_size[2], unsigned *font_size);

struct Pd_MiniparserException : public std::runtime_error {
  using std::runtime_error::runtime_error;
};
