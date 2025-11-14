#include "diskstat.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include "data_local.h"
#include "conky_format.h"


int read_device_paths(const std::string& file_path,
                      std::vector<std::string>& lines) {
  std::ifstream file(file_path);
  std::string line;

  namespace fs = std::filesystem;

  // Check if the file exists, is a regular file, and is readable
  if (!fs::exists(file_path) || !fs::is_regular_file(file_path)) {
    std::cerr << "Unable to load file: " + file_path << std::endl;
    return 1;
  }

  while (std::getline(file, line)) {
    if (!line.empty()) lines.push_back(line);
  }
  return 0;
}

void diskstat(const std::string& config_file) {
  LocalDataStreams provider;
  diskstat(provider, config_file);
}

int diskstat(DataStreamProvider& provider, const std::string& config_file) {

  extern std::tuple<std::string, std::function<FuncType>> conky_columns[];
  extern const size_t CONKY_COLUMNS_COUNT;

  std::vector<std::string> device_paths;
  if (read_device_paths(config_file, device_paths) == 1) {
    std::cout << "${color red}Unable to load file: " + config_file + "${color}"
              << std::endl;
    return 1;
  }
  std::vector<DeviceInfo> devices = collect_device_info(provider, device_paths);
  std::cout.setf(std::ios::unitbuf);

  print_column_headers(conky_columns, CONKY_COLUMNS_COUNT);
  print_rows(devices, CONKY_COLUMNS_COUNT);
  return 0;
}
