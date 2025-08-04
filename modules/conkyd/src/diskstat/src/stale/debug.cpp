// Quick debug version to see what's happening
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

int main() {
  std::cout << "All mounted devices:" << std::endl;

  std::ifstream mounts("/proc/mounts");
  std::string device, mount_point, rest;

  while (mounts >> device >> mount_point) {
    std::getline(mounts, rest);
    if (device.find("/dev/mapper/bork") != std::string::npos ||
        device.find("/dev/system") != std::string::npos) {
      std::cout << device << " -> " << mount_point << std::endl;
    }
  }

  return 0;
}
