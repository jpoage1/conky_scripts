#include "io.hpp"

#include <array>
#include <ctime>
#include <fstream>

#include "rotate_file_sync.hpp"
#include "teebuf.hpp"

TeeBuf _kout_buf;
TeeBuf _kerr_buf;

std::ostream kout(&_kout_buf);
std::ostream kerr(&_kerr_buf);

// Persistent file handles
std::ofstream log_file("logs/system.log", std::ios::app);
std::ofstream err_file("logs/error.log", std::ios::app);
// Update global storage to use the custom sink
std::unique_ptr<RotatingFileSink> global_out_file;
std::unique_ptr<RotatingFileSink> global_err_file;

void setup_io(std::streambuf* out_socket, std::streambuf* err_socket) {
  _kout_buf.clear_sinks();
  _kerr_buf.clear_sinks();

  // LLM-Optimized Config
  bool rotate = true;
  unsigned int max_logs = 10;  // You requested only one iteration of testing

  // ISO-8601 Compact: 20251220T0137Z
  // std::string ts_fmt = "%Y-%m-%d %H:%M:%S";
  // 2. Define the log entry template (%s is the placeholder for timestamp)
  // std::string out_template = "[%s] [INFO] ";
  // std::string err_template = "[%s] [ERROR] ";

  // strftime compatible
  std::string ts_fmt = "%Y%m%dT%H%M%S";

  // Schema: TS=[timestamp]|LVL=[level]|MSG=
  // This allows the LLM to anchor on "MSG=" for content extraction.
  std::string out_template = "TS=%s|LVL=INFO|MSG=";
  std::string err_template = "TS=%s|LVL=ERR|MSG=";

  global_out_file = std::make_unique<RotatingFileSink>(
      "logs/system.log", rotate, max_logs, ts_fmt, out_template);
  global_err_file = std::make_unique<RotatingFileSink>(
      "logs/error.log", rotate, max_logs, ts_fmt, err_template);

  // Attach custom sinks to the TeeBuf
  _kout_buf.add_sink(std::cout.rdbuf());
  _kout_buf.add_sink(global_out_file.get());
  if (out_socket) _kout_buf.add_sink(out_socket);

  _kerr_buf.add_sink(std::cerr.rdbuf());
  _kerr_buf.add_sink(global_err_file.get());
  if (err_socket) _kerr_buf.add_sink(err_socket);
}
