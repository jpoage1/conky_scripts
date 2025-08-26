
#include <iostream>
int diskstat(const std::string &config_file);

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <device_list_file>\n";
    return 1;
  }
  return diskstat(argv[1]);
}
