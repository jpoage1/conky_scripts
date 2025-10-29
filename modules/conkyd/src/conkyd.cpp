// conkyd.cpp

#include <iostream>

#include "metrics.hpp"

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <device_list_file>\n";
    return 1;
  }
  return get_metrics(argv[1], true);
}
