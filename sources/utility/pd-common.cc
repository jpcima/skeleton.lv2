#include "pd-common.h"
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/null.hpp>
#include <iostream>
namespace io = boost::iostreams;

bool pd_verbose = false;

std::ostream &pd_logs() {
  static io::stream<io::null_sink> nulls{io::null_sink()};
  return pd_verbose ? std::cerr : nulls;
}
