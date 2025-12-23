#ifndef ROTATE_FILE_SYNC
#define ROTATE_FILE_SYNC

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <vector>

namespace fs = std::filesystem;

class RotatingFileSink : public std::streambuf {
 public:
  explicit RotatingFileSink(fs::path path, bool rotate, unsigned int max_files,
                            std::string ts_fmt, std::string entry_fmt);
  ~RotatingFileSink() override;

 protected:
  int_type overflow(int_type c) override;
  int sync() override;

 private:
  void rotate_logs();
  std::string get_formatted_prefix();

  std::ofstream file_stream;
  fs::path base_path;
  bool do_rotate;
  unsigned int log_num;
  std::string timestamp_fmt;
  std::string log_fmt;
  bool at_line_start = true;
  std::mutex mtx;
};

#endif
