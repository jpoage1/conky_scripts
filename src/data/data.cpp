// data.cpp
#include "data.hpp"

#include <sys/utsname.h>

#include "corestat.hpp"
#include "cpuinfo.hpp"
#include "diskstat.hpp"
#include "filesystems.hpp"
#include "hwmonitor.hpp"
#include "load_avg.hpp"
#include "meminfo.hpp"
#include "metrics.hpp"
#include "networkstats.hpp"
#include "parser.hpp"
#include "polling.hpp"
#include "processinfo.hpp"
#include "sysinfo.hpp"
#include "uptime.hpp"

void dump_fstream(std::istream& stream) {
  std::stringstream buffer;
  buffer << stream.rdbuf();
  std::cerr << "Dumping buffer: " << std::endl;
  std::cerr << buffer.str() << std::endl;
  std::cerr << "Done" << std::endl;
}
IPollingTask::IPollingTask(DataStreamProvider& _provider,
                           SystemMetrics& _metrics, MetricsContext& context)
    : provider(_provider), metrics(_metrics) {
  //   std::cerr << "Parent constructor called" << std::endl;
  //   dump_fstream(provider.get_stat_stream());
  //   std::cerr << "End of Parent constructor call" << std::endl;
}
void log_stream_state(const std::string time, const std::string stream_name,
                      const std::string state) {
  std::cerr << "DEBUG: Stream " << stream_name << "was in " << state
            << " state " << time << " rewind." << std::endl;
}
// void log_stream_state(const std::string time, const std::string stream_name,
//                       const std::string state) {
//   // This function was just a helper. We can approximate its
//   // original behavior by calling the new macros.
//   std::string msg = "Stream " + stream_name + " was in " + state + " state "
//   +
//                     time + " rewind.";

//   if (state == "bad" || state == "fail") {
//     LOG_WARNING(msg);  // Will respect global log level
//   } else {
//     LOG_NOTICE(msg);  // Will respect global log level
//   }
// }
void log_stream_state(const std::istream& stream, const LogLevel log_level,
                      const std::string time, const std::string stream_name) {
  if (log_level == LogLevel::None) return;
  const std::string streamName =
      stream_name != "" ? "`" + stream_name + "` " : "";
  if (log_level == LogLevel::Debug) {
    std::cerr << "DEBUG STATE for " << streamName
              << ":"
              // good() is true only if no flags are set
              << " good()=" << std::boolalpha
              << stream.good()
              // eof() is true if end-of-file was reached
              << " eof()=" << std::boolalpha
              << stream.eof()
              // fail() is true on formatting errors or if stream isn't open
              << " fail()=" << std::boolalpha
              << stream.fail()
              // bad() is true on unrecoverable read/write errors
              << " bad()=" << std::boolalpha << stream.bad() << std::endl;
    return;
  }
  if (stream.good() && log_level == LogLevel::Debug) {
    log_stream_state(time, streamName, "good");
  } else if (stream.bad() && log_level == LogLevel::Warning) {
    log_stream_state(time, streamName, "bad");
  } else if (stream.eof() && log_level == LogLevel::Notice) {
    log_stream_state(time, streamName, "eof");
  } else if (stream.fail() && log_level == LogLevel::Notice) {
    log_stream_state(time, streamName, "fail");
  }
}
void log_stream_state(std::istream& stream, const LogLevel log_level,
                      const std::string time) {
  const std::string stream_name = "";
  log_stream_state(stream, log_level, time, stream_name);
}
void DataStreamProvider::rewind(std::istream& stream, std::string stream_name) {
  LogLevel log_level = LogLevel::Warning;
  log_stream_state(stream, log_level, static_cast<std::string>("before"),
                   stream_name);

  stream.clear();
  stream.seekg(0, std::ios::beg);

  log_stream_state(stream, log_level, static_cast<std::string>("after"),
                   stream_name);
}
void DataStreamProvider::rewind(std::istream& stream) { rewind(stream, ""); }
