#include "rotate_file_sync.hpp"

#include <array>
#include <ctime>
#include <fstream>

#include "io.hpp"
namespace telemetry {
static thread_local bool is_logging = false;

std::string RotatingFileSink::get_formatted_prefix() {
  auto now = std::chrono::system_clock::now();
  std::time_t now_c = std::chrono::system_clock::to_time_t(now);

  // Use a fixed-size buffer for the timestamp to avoid heap churn
  std::array<char, 20> time_buf;
  if (std::strftime(time_buf.data(), time_buf.size(), timestamp_fmt.c_str(),
                    std::gmtime(&now_c)) == 0) {
    return "TS=0|MSG=";
  }

  // Apply the template. Replacing "%s" in a pre-allocated string.
  std::string result = log_fmt;
  size_t pos = result.find("%s");
  if (pos != std::string::npos) {
    result.replace(pos, 2, time_buf.data());
  }
  return result;
}
std::streambuf::int_type RotatingFileSink::overflow(int_type c) {
  if (c == traits_type::eof())
    return traits_type::not_eof(c);

  // Prevent recursion
  if (is_logging)
    return c;
  is_logging = true;

  std::lock_guard<std::mutex> lock(mtx);

  try {
    if (at_line_start) {
      std::string prefix = get_formatted_prefix();
      file_stream.write(prefix.c_str(),
                        static_cast<std::streamsize>(prefix.length()));
      at_line_start = false;
    }

    char ch = traits_type::to_char_type(c);
    file_stream.put(ch);

    if (ch == '\n') {
      at_line_start = true;
      file_stream.flush();
    }
  } catch (...) {
    // Suppress errors to avoid crashing the whole system during I/O failure
  }

  is_logging = false;
  return c;
}
#include <system_error>

RotatingFileSink::RotatingFileSink(fs::path path, bool rotate,
                                   unsigned int max_files, std::string ts_fmt,
                                   std::string entry_fmt)
    : base_path(std::move(path)), do_rotate(rotate), log_num(max_files),
      timestamp_fmt(std::move(ts_fmt)), log_fmt(std::move(entry_fmt)) {
  if (do_rotate) {
    rotate_logs();
  }

  std::error_code ec;
  fs::create_directories(base_path.parent_path(), ec);

  file_stream.open(
      base_path, std::ios::out | (do_rotate ? std::ios::trunc : std::ios::app));
}

RotatingFileSink::~RotatingFileSink() {
  if (file_stream.is_open()) {
    file_stream.flush();
    file_stream.close();
  }
}

void RotatingFileSink::rotate_logs() {
  std::error_code ec;
  if (!fs::exists(base_path, ec))
    return;

  for (int i = static_cast<int>(log_num) - 1; i >= 1; --i) {
    fs::path old_log = base_path.string() + "." + std::to_string(i);
    fs::path new_log = base_path.string() + "." + std::to_string(i + 1);

    if (fs::exists(old_log, ec)) {
      if (static_cast<unsigned int>(i + 1) > log_num) {
        fs::remove(old_log, ec);
      } else {
        fs::rename(old_log, new_log, ec);
      }
    }
  }

  fs::rename(base_path, base_path.string() + ".1", ec);
}

int RotatingFileSink::sync() {
  std::lock_guard<std::mutex> lock(mtx);
  if (file_stream.is_open()) {
    file_stream.flush();
  }
  return 0;
}
}; // namespace telemetry
