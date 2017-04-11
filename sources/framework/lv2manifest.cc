#include "description.h"
#include "lv2all.h"
#include <boost/locale/encoding_utf.hpp>
#include <boost/utility/string_view.hpp>
#include <iterator>
#include <string>
#include <fstream>

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
  const UIManifest &uim = ::ui_manifest;

  std::string effect_ttl_file = cat2(effect_binary_file, ".ttl");
  std::string ui_ttl_file = cat2(ui_binary_file, ".ttl");

  std::ofstream ttl(cat2(directory, "/manifest.ttl"), std::ios::binary);
  std::ofstream fxttl(cat3(directory, "/", effect_ttl_file), std::ios::binary);
  std::ofstream uittl(cat3(directory, "/", ui_ttl_file), std::ios::binary);

  //============================================================================
  write_prefix(ttl);

  ttl << ttl_uri(fxm.uri) << "\n";
  ttl << "  a lv2:Plugin ;\n";
  ttl << "  lv2:binary " << ttl_uri(effect_binary_file) << " ;\n";
  ttl << "  rdfs:seeAlso " << ttl_uri(effect_ttl_file) << " .\n";

  ttl << ttl_uri(uim.uri) << "\n";
  ttl << "  a " << ttl_uri(uim.uiclass) << " ;\n";
  ttl << "  lv2:binary " << ttl_uri(ui_binary_file) << " ;\n";
  ttl << "  rdfs:seeAlso " << ttl_uri(ui_ttl_file) << " .\n";

  //============================================================================
  write_effect_manifest(fxm, fxttl);
  write_ui_manifest(uim, uittl);

  //============================================================================
  ttl.flush();
  fxttl.flush();
  uittl.flush();

  return ttl.good() && fxttl.good() && uittl.good();
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

  ttl << ttl_uri(m.uri) << "\n";
  ttl << "  a lv2:Plugin";
  for (const std::string &cat : m.categories) ttl << ", " << ttl_uri(cat);
  ttl << " ;\n";
  ttl << "  doap:name " << ttl_string(m.name) << " ;\n";
  for (const FeatureRequest &f : m.features)
    ttl << "  lv2:" << (f.required ? "required" : "optional") << "Feature "
        << ttl_uri(f.uri) << " ;\n";
  for (const std::string &e : m.extension_data)
    ttl << "lv2:extensionData " << ttl_uri(e) << " ;\n";

  ttl << "  ui:ui " << ttl_uri(ui_uri) << " ;\n";

  ttl << "  lv2:port [\n";
  for (size_t i = 0, n = m.ports.size(); i < n; ++i) {
    if (i > 0)
      ttl << "  ] , [\n";

    const Port &port = *m.ports[i];
    const PortKind kind = port.kind();

    ttl << "    a lv2:" << port.direction._to_string() << "Port";
    switch (kind) {
      case PortKind::Audio: ttl << ", lv2:AudioPort"; break;
      case PortKind::Control: ttl << ", lv2:ControlPort"; break;
      case PortKind::Event: ttl << ", atom:AtomPort" ; break;
    }
    ttl << " ;\n";

    ttl << "    lv2:index " << i << " ; \n";
    ttl << "    lv2:symbol " << ttl_string(port.symbol) << " ; \n";
    ttl << "    lv2:name " << ttl_string(port.name) << " ; \n";
    if (!port.designation.empty())
      ttl << "    lv2:designation " << ttl_uri(port.designation) << " ; \n";

    switch (kind) {
      case PortKind::Control: {
        const ControlPort &cp = static_cast<const ControlPort &>(port);
        ttl << "    lv2:default " << cp.default_value << " ;\n";
        ttl << "    lv2:minimum " << cp.minimum_value << " ;\n";
        ttl << "    lv2:maximum " << cp.maximum_value << " ;\n";
        break;
      }
      case PortKind::Event: {
        const EventPort &ep = static_cast<const EventPort &>(port);
        if (!ep.buffer_type.empty())
          ttl << "    atom:bufferType " << ttl_uri(ep.buffer_type) << " ;\n";
        if (!ep.supports.empty())
          ttl << "    atom:supports " << ttl_uri(ep.supports) << " ;\n";
        break;
      }
      default:
        break;
    }
  }
  ttl << "  ] .\n";
}

static void write_ui_manifest(const UIManifest &m, std::ostream &ttl) {
  write_prefix(ttl);

  ttl << ttl_uri(m.uri) << "\n";
  ttl << "  a " << ttl_uri(m.uiclass) << " ;\n";

  for (const FeatureRequest &f : m.features)
    ttl << "  lv2:" << (f.required ? "required" : "optional") << "Feature "
        << ttl_uri(f.uri) << " ;\n";
  for (const std::string &e : m.extension_data)
    ttl << "  lv2:extensionData " << ttl_uri(e) << " ;\n";

  ttl << "  ui:portNotification [\n";
  // TODO: port notifications
  ttl << "  ] .\n";
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
