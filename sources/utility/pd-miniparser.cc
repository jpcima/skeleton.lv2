#include "pd-miniparser.h"
#include <boost/algorithm/string/split.hpp>
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <iostream>

std::vector<std::string> pd_read_records(std::istream &in) {
  std::vector<std::string> records;

  std::string buffer;
  buffer.reserve(1024);

  auto check_bad = [&]() {
    if (in.bad()) throw Pd_MiniparserException("Input error reading pd patch"); };
  auto check_fail = [&](const char *msg) {
    check_bad();
    if (in.fail()) throw Pd_MiniparserException(msg); };

  auto get = [&]() -> int {
    int c = in.get();
    check_bad();
    return c;
  };
  auto ensure_get = [&]() -> int {
    int c = in.get();
    check_fail("Premature end reading pd patch");
    return c;
  };

  for (int ch; (ch = get()) != EOF;) {
    if (ch != '#')
      throw Pd_MiniparserException(
          "invalid character at beginning of record, '#' expected");

    buffer.push_back(ch);
    while ((ch = ensure_get()) != ';') {
      if (ch == '\\') ch = ensure_get();
      buffer.push_back(ch);
    }

    ch = ensure_get();
    if (ch == '\r')
      ch = ensure_get();
    if (ch != '\n')
      throw Pd_MiniparserException(
          "invalid character at end of record, line break expected");

    records.push_back(buffer);
    buffer.clear();
  }

  check_bad();
  return records;
}

std::vector<std::string> pd_file_read_records(boost::string_view path) {
  std::ifstream in(path.to_string(), std::ios::binary);
  return pd_read_records(in);
}

bool pd_record_parse_obj(
    const std::string &record_,
    int *x, int *y, std::string *exp) {
  record_.c_str();
  boost::string_view record(record_);

  int idummy;
  if (!x) x = &idummy;
  if (!y) y = &idummy;

  boost::string_view prefix = "#X obj ";
  if (!record.starts_with(prefix))
    return false;
  record.remove_prefix(prefix.size());

  unsigned count {};
  if (sscanf(record.data(), "%d %d %n", x, y, &count) != 2)
    return false;
  record.remove_prefix(count);

  if (exp)
    *exp = record.to_string();
  return true;
}

bool pd_record_parse_root_canvas(
    const std::string &record,
    int pos[2], unsigned size[2], unsigned *fs) {
  int idummy[2];
  if (!pos) pos = idummy;
  if (!size) size = (unsigned *)idummy;
  if (!fs) fs = (unsigned *)idummy;

  unsigned count {};
  if (sscanf(record.data(), "#N canvas %d %d %u %u %u%n",
             &pos[0], &pos[1], &size[0], &size[1], fs, &count) != 5 ||
      count != record.size())
    return false;

  return true;
}

boost::string_view pd_exp_cmd(const std::string &exp_) {
  boost::string_view exp(exp_);
  return exp.substr(0, exp.find(' '));
}

std::vector<std::string> pd_exp_tokens(const std::string &exp) {
  std::vector<std::string> tokens;
  boost::split(tokens, exp, [](int c) { return c == ' '; });
  return tokens;
}

static bool pd_exp_parse_adcdac(
    const std::vector<std::string> &tokens, std::vector<int> &channels) {
  size_t n = tokens.size() - 1;
  if (n == 0) {
    channels = {0, 2};
  } else {
    channels.resize(n);
    for (size_t i = 0; i < n; ++i) {
      int ch {};
      if (boost::conversion::try_lexical_convert<int>(tokens[i + 1], ch))
        ch = (ch > 0) ? (ch - 1) : -1;
      else
        ch = -1;
      channels[i] = ch;
    }
  }
  return true;
}

bool pd_exp_parse_adc(const std::string &exp, std::vector<int> &channels) {
  std::vector<std::string> tokens = pd_exp_tokens(exp);
  return pd_exp_cmd(exp) == "adc~" && pd_exp_parse_adcdac(tokens, channels);
}

bool pd_exp_parse_dac(const std::string &exp, std::vector<int> &channels) {
  std::vector<std::string> tokens = pd_exp_tokens(exp);
  return pd_exp_cmd(exp) == "dac~" && pd_exp_parse_adcdac(tokens, channels);
}

bool pd_patch_getinfo(
    const std::vector<std::string> &records,
    unsigned *adc_count, unsigned *dac_count,
    bool *has_midi_in, bool *has_midi_out,
    int root_canvas_pos[2], unsigned root_canvas_size[2], unsigned *font_size) {
  if (records.empty())
    return false;

  if (!pd_record_parse_root_canvas(
          records.front(), root_canvas_pos, root_canvas_size, font_size))
    return false;

  std::vector<int> channels;
  channels.reserve(32);

  std::string obj_exp;
  obj_exp.reserve(256);

  static boost::string_view midi_in_cmds[] = {
    "notein", "ctlin", "pgmin", "bendin", "touchin", "polytouchin",
    "midiin", "sysexin", "midirealtimein", "midiclkin" };
  static boost::string_view midi_out_cmds[] = {
    "noteout", "ctlout", "pgmout", "bendout", "touchout", "polytouchout",
    "midiout" };

  for (const std::string &record : records) {
    if (pd_record_parse_obj(record, nullptr, nullptr, &obj_exp)) {
      boost::string_view cmd = pd_exp_cmd(obj_exp);

      if (pd_exp_parse_adc(obj_exp, channels)) {
        for (int channel : channels)
          if (channel >= 0)
            *adc_count = std::max(*adc_count, channel + 1u);
      }
      if (pd_exp_parse_dac(obj_exp, channels)) {
        for (int channel : channels)
          if (channel >= 0)
            *dac_count = std::max(*dac_count, channel + 1u);
      }

      if (has_midi_in && !*has_midi_in) {
        auto B = std::begin(midi_in_cmds), E = std::end(midi_in_cmds);
        *has_midi_in = std::find(B, E, cmd) != E;
      }
      if (has_midi_out && !*has_midi_out) {
        auto B = std::begin(midi_out_cmds), E = std::end(midi_out_cmds);
        *has_midi_out = std::find(B, E, cmd) != E;
      }
    }
  }

  return true;
}
