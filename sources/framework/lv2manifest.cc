#include "description.h"
#include "lv2all.h"
#include <boost/locale/encoding_utf.hpp>
#include <boost/utility/string_view.hpp>
#include <iterator>
#include <string>
#include <fstream>
#include <cassert>

//==============================================================================
static void write_prefix(std::ostream &ttl);
static void write_effect_manifest(const EffectManifest &m, std::ostream &ttl);
static void write_ui_manifest(const UIManifest &m, std::ostream &ttl);

//==============================================================================
static std::string ttl_uri(boost::string_view uri);
static std::string ttl_string(boost::string_view uri);

//==============================================================================
static std::string catN(const boost::string_view *a, unsigned n);
static std::string cat2(boost::string_view a, boost::string_view b);
static std::string cat3(boost::string_view a, boost::string_view b, boost::string_view c);

//==============================================================================
LV2_SYMBOL_EXPORT
bool lv2_write_manifest(const char *directory) {
  const EffectManifest &fxm = ::effect_manifest;
  const boost::optional<UIManifest> &opt_uim = ::ui_manifest;

  std::string effect_ttl_file = cat2(effect_binary_file, ".ttl");
  std::string ui_ttl_file = cat2(ui_binary_file, ".ttl");

  //============================================================================
  std::ofstream ttl(cat2(directory, "/manifest.ttl"), std::ios::binary);
  write_prefix(ttl);

  ttl << ttl_uri(fxm.uri) << "\n";
  ttl << "  a lv2:Plugin ;\n";
  ttl << "  lv2:binary " << ttl_uri(effect_binary_file) << " ;\n";
  ttl << "  rdfs:seeAlso " << ttl_uri(effect_ttl_file) << " .\n";

  if (opt_uim) {
    const UIManifest &uim = *opt_uim;
    ttl << ttl_uri(uim.uri) << "\n";
    ttl << "  a " << ttl_uri(uim.uiclass) << " ;\n";
    ttl << "  lv2:binary " << ttl_uri(ui_binary_file) << " ;\n";
    ttl << "  rdfs:seeAlso " << ttl_uri(ui_ttl_file) << " .\n";
  }

  if (!ttl.flush())
    return false;

  //============================================================================
  std::ofstream fxttl(cat3(directory, "/", effect_ttl_file), std::ios::binary);
  write_effect_manifest(fxm, fxttl);
  if (!fxttl.flush())
    return false;

  if (opt_uim) {
    const UIManifest &uim = *opt_uim;
    std::ofstream uittl(cat3(directory, "/", ui_ttl_file), std::ios::binary);
    write_ui_manifest(uim, uittl);
    if (!uittl.flush())
      return false;
  }

  //============================================================================
  return true;
}

static void write_prefix(std::ostream &ttl) {
  ttl <<
      "@prefix doap:  <http://usefulinc.com/ns/doap#> .\n"
      "@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#> .\n"
      "@prefix lv2:  <" LV2_CORE_PREFIX "> .\n"
      "@prefix atom: <" LV2_ATOM_PREFIX "> .\n"
      "@prefix ui:   <" LV2_UI_PREFIX "> .\n";
}

static void write_effect_manifest(const EffectManifest &m, std::ostream &ttl) {
  write_prefix(ttl);

  ttl << "\n" << ttl_uri(m.uri);
  ttl << "\n  a lv2:Plugin";
  for (const std::string &cat : m.categories) ttl << ", " << ttl_uri(cat);
  ttl << " ;";
  ttl << "\n  doap:name " << ttl_string(m.name) << " ;";
  for (const FeatureRequest &f : m.features)
    ttl << "\n  lv2:" << (bool(f.required) ? "required" : "optional")
        << "Feature " << ttl_uri(f.uri) << " ;";
  for (const std::string &e : m.extension_data)
    ttl << "\n  lv2:extensionData " << ttl_uri(e) << " ;";

  ttl << "\n  ui:ui " << ttl_uri(ui_uri) << " ;";

  for (size_t i = 0, n = m.ports.size(); i < n; ++i) {
    ttl << "\n  lv2:port [";

    const Port &port = *m.ports[i];
    const PortKind kind = port.kind();

    ttl << "\n    a lv2:";
    switch (port.direction) {
      case PortDirection::Input: ttl << "InputPort"; break;
      case PortDirection::Output: ttl << "OutputPort"; break;
      default: assert(false);
    }
    switch (kind) {
      case PortKind::Audio: ttl << ", lv2:AudioPort"; break;
      case PortKind::Control: ttl << ", lv2:ControlPort"; break;
      case PortKind::Event: ttl << ", atom:AtomPort" ; break;
      default: assert(false);
    }
    ttl << " ;";

    ttl << "\n    lv2:index " << i << " ;";
    ttl << "\n    lv2:symbol " << ttl_string(port.symbol) << " ;";
    ttl << "\n    lv2:name " << ttl_string(port.name) << " ;";
    if (!port.designation.empty())
      ttl << "\n    lv2:designation " << ttl_uri(port.designation) << " ;";

    switch (kind) {
      case PortKind::Control: {
        const ControlPort &cp = static_cast<const ControlPort &>(port);
        ttl << "\n    lv2:default " << cp.default_value << " ;";
        ttl << "\n    lv2:minimum " << cp.minimum_value << " ;";
        ttl << "\n    lv2:maximum " << cp.maximum_value << " ;";
        break;
      }
      case PortKind::Event: {
        const EventPort &ep = static_cast<const EventPort &>(port);
        if (!ep.buffer_type.empty())
          ttl << "\n    atom:bufferType " << ttl_uri(ep.buffer_type) << " ;";
        if (!ep.supports.empty()) {
          ttl << "\n    atom:supports " << ttl_uri(ep.supports[0]);
          for (unsigned i = 1, n = ep.supports.size(); i < n; ++i)
            ttl << ", " << ttl_uri(ep.supports[i]);
          ttl << " ;";
        }
        break;
      }
      default:
        break;
    }
    ttl << "\n  ] ;";
  }

  ttl.seekp(-1, std::ios::cur);
  ttl << ".\n";
}

static void write_ui_manifest(const UIManifest &m, std::ostream &ttl) {
  write_prefix(ttl);

  ttl << "\n" << ttl_uri(m.uri);
  ttl << "\n  a " << ttl_uri(m.uiclass) << " ;";

  for (const FeatureRequest &f : m.features)
    ttl << "\n  lv2:" << (bool(f.required) ? "required" : "optional")
        << "Feature " << ttl_uri(f.uri) << " ;";
  for (const std::string &e : m.extension_data)
    ttl << "\n  lv2:extensionData " << ttl_uri(e) << " ;";
  for (const PortNotification &pn : m.port_notifications) {
    ttl << "\n  ui:portNotification ["
        << "\n    ui:plugin " << ttl_uri(m.effect_uri) << " ;"
        << "\n    lv2:symbol " << ttl_string(pn.symbol) << " ;";
    if (!pn.protocol.empty())
      ttl << "\n    ui:protocol " << ttl_uri(pn.protocol) << " ;";
    if (!pn.notify_type.empty())
      ttl << "\n    ui:notifyType " << ttl_uri(pn.notify_type) << " ;";
    ttl << "\n  ] ;";
  }

  ttl.seekp(-1, std::ios::cur);
  ttl << ".\n";
}

//==============================================================================
static std::string catN(const boost::string_view *a, unsigned n) {
  size_t size = 0;
  for (unsigned i = 0; i < n; ++i)
    size += a[i].size();
  std::string r;
  r.reserve(size);
  for (unsigned i = 0; i < n; ++i)
    r.append(a[i].data(), a[i].size());
  return r;
}

static std::string cat2(boost::string_view a, boost::string_view b) {
  boost::string_view s[] {a, b};
  return catN(s, 2);
}

static std::string cat3(boost::string_view a, boost::string_view b, boost::string_view c) {
  boost::string_view s[] {a, b, c};
  return catN(s, 3);
}

//==============================================================================
static std::string ttl_uri(boost::string_view uri) {
  using boost::locale::conv::utf_to_utf;
  using boost::locale::utf::utf_traits;

  std::string out;
  out.reserve(2 + 2 * uri.size());

  std::back_insert_iterator<std::string> it(out);
  auto add = [&](char32_t c) { utf_traits<char>::encode(c, it); };

  const std::u32string unicode = utf_to_utf<char32_t>(
      uri.data(), uri.data() + uri.size());
  add('<');
  for (const char32_t c : unicode) {
    switch (c) {
      case U'\t': add(U'\\'); add(U't'); break;
      case U'\n': add(U'\\'); add(U'n'); break;
      case U'\r': add(U'\\'); add(U'r'); break;
      case U'>':
      case U'\\': add(U'\\'); // fall through
      default: add(c); break;
    }
  }
  add('>');
  return out;
}

static std::string ttl_string(boost::string_view text) {
  using boost::locale::conv::utf_to_utf;
  using boost::locale::utf::utf_traits;

  std::string out;
  out.reserve(2 + 2 * text.size());

  std::back_insert_iterator<std::string> it(out);
  auto add = [&](char32_t c) { utf_traits<char>::encode(c, it); };

  const std::u32string unicode = utf_to_utf<char32_t>(
      text.data(), text.data() + text.size());
  add('"');
  for (const char32_t c : unicode) {
    switch (c) {
      case U'\t': add(U'\\'); add(U't'); break;
      case U'\n': add(U'\\'); add(U'n'); break;
      case U'\r': add(U'\\'); add(U'r'); break;
      case U'"':
      case U'\\': add(U'\\'); // fall through
      default: add(c); break;
    }
  }
  add('"');
  return out;
}
