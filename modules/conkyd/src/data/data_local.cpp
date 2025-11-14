
#include "data_local.h"

#include <array>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>

void LocalDataStreams::reset_stream(std::ifstream& stream,
                                    const std::string& path) {
  if (stream.is_open()) {
    stream.close();
  }
  stream.open(path);
  if (!stream.is_open()) {
    std::cerr << "[Error] Failed to open " << path << std::endl;
  }
}

std::string LocalDataStreams::exec_local_cmd(const char* cmd) {
  std::array<char, 128> buffer;
  std::string result;
  // Use unique_ptr with the custom deleter struct
  // This avoids the decltype warning
  std::unique_ptr<FILE, PopenDeleter> pipe(popen(cmd, "r"));

  if (!pipe) {
    std::cerr << "popen() failed for command: " << cmd << std::endl;
    return "";
  }
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }
  return result;
}

std::optional<std::string> LocalDataStreams::read_sysfs_file(
    const std::filesystem::path& path) {
  if (!std::filesystem::exists(path)) {
    return std::nullopt;
  }
  std::ifstream file(path);
  if (!file.is_open()) {
    return std::nullopt;
  }
  std::string line;
  if (std::getline(file, line)) {
    return line;
  }
  return std::nullopt;
}
