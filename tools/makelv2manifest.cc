#include <string>
#include <stdexcept>
#include <iostream>
#if defined(_WIN32)
# include <windows.h>
#else
# include <dlfcn.h>
#endif

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: makelv2manifest <input-file> <output-dir>\n";
    return 1;
  }

  std::string inputfile = argv[1];
  std::string outputdir = argv[2];

  if (inputfile.empty() || outputdir.empty())
    return 1;

#if !defined(_WIN32)
  if (inputfile.front() != '/')
    inputfile = "./" + inputfile;
#endif

#if defined(_WIN32)
  HMODULE dlh = LoadLibraryA(inputfile.c_str());
#else
  void *dlh = dlopen(inputfile.c_str(), RTLD_LAZY);
#endif
  if (!dlh)
    throw std::runtime_error("cannot load the library");

  typedef bool (fn_t)(const char *, bool);
#if defined(_WIN32)
  fn_t *fn = (fn_t *)GetProcAddress(dlh, "lv2_write_manifest");
#else
  fn_t *fn = (fn_t *)dlsym(dlh, "lv2_write_manifest");
#endif
  if (!fn)
    throw std::runtime_error("cannot find the entry function");

  bool single_file = false;
  if (!fn(outputdir.c_str(), single_file))
    throw std::runtime_error("error writing the manifest");

  return 0;
}
