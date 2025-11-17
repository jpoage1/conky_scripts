
#include "data_local.hpp"
void LocalDataStreams::finally() {}
std::ifstream& LocalDataStreams::create_stream_from_file(
    std::ifstream& stream, const std::string& path) {
  if (stream.is_open()) {
    // std::cerr << "Stream is already open" << std::endl;
    stream.close();
  } else {
    // std::cerr << "Stream is not already open" << std::endl;
  }
  stream.clear();
  stream.open(path);
  if (!stream.is_open()) {
    std::cerr << "[Error] Failed to open " << path << std::endl;
  }
  return stream;
}

std::stringstream& LocalDataStreams::create_stream_from_command(
    std::stringstream& stream, const char* cmd) {
  return create_stream_from_command(stream, cmd, static_cast<std::string>(cmd));
}
std::stringstream& LocalDataStreams::create_stream_from_command(
    std::stringstream& stream, const char* cmd, std::string stream_name) {
  std::string cmd_output = exec_local_cmd(cmd);

  stream.str(std::move(cmd_output));
  rewind(stream, stream_name);
  return stream;
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
