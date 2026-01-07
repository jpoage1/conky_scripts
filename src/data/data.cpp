// data.cpp
#include <sys/utsname.h>

#include "cli_parser.hpp"
#include "corestat.hpp"
#include "cpuinfo.hpp"
#include "data_local.hpp"
#include "diskstat.hpp"
#include "filesystems.hpp"
#include "hwmonitor.hpp"
#include "load_avg.hpp"
#include "lua_generator.hpp"
#include "meminfo.hpp"
#include "metric_settings.hpp"
#include "metrics.hpp"
#include "networkstats.hpp"
#include "polling.hpp"
#include "processinfo.hpp"
#include "provider.hpp"
#include "stream_provider.hpp"
#include "sysinfo.hpp"
#include "uptime.hpp"

namespace telemetry {

void dump_fstream(std::istream &stream) {
  std::stringstream buffer;
  buffer << stream.rdbuf();
  std::cerr << "Dumping buffer: " << std::endl;
  std::cerr << buffer.str() << std::endl;
  std::cerr << "Done" << std::endl;
}
IPollingTask::IPollingTask(DataStreamProvider &_provider,
                           SystemMetrics &_metrics,
                           MetricsContext & /*context*/)
    : provider(_provider), metrics(_metrics) {
  //   dump_fstream(provider.get_stat_stream());
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
void log_stream_state(const std::istream &stream, const LogLevel log_level,
                      const std::string time, const std::string stream_name) {
  if (log_level == LogLevel::None)
    return;
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
void log_stream_state(std::istream &stream, const LogLevel log_level,
                      const std::string time) {
  const std::string stream_name = "";
  log_stream_state(stream, log_level, time, stream_name);
}
void DataStreamProvider::rewind(std::istream &stream, std::string stream_name) {
  LogLevel log_level = LogLevel::Warning;
  log_stream_state(stream, log_level, static_cast<std::string>("before"),
                   stream_name);

  stream.clear();
  stream.seekg(0, std::ios::beg);

  log_stream_state(stream, log_level, static_cast<std::string>("after"),
                   stream_name);
}
void DataStreamProvider::rewind(std::istream &stream) { rewind(stream, ""); }

std::string LuaProviderSettings::serialize(int indentation_level) const {
  LuaConfigGenerator gen(indentation_level); // Anonymous table
  gen.lua_string("type", type);
  gen.lua_string("name", name);
  gen.lua_bool("enabled", enabled);

  // Sub-table for custom provider parameters
  std::stringstream params_ss;
  for (auto const &[key, val] : parameters) {
    LuaConfigGenerator p_gen(indentation_level + 2);
    p_gen.lua_string(key, val);
    params_ss << p_gen.raw_str();
  }
  gen.lua_append(gen.lua_mklist("parameters", params_ss.str()));

  return gen.str();
}

void LuaProviderSettings::deserialize(const sol::table &provider_table) {
  if (!provider_table.valid())
    return;

  type = provider_table.get_or("type", std::string("local"));
  name = provider_table.get_or("name", std::string("default"));
  enabled = provider_table.get_or("enabled", true);

  if (provider_table["parameters"].valid()) {
    sol::table params = provider_table["parameters"];
    for (auto &kv : params) {
      if (kv.first.is<std::string>() && kv.second.is<std::string>()) {
        parameters[kv.first.as<std::string>()] = kv.second.as<std::string>();
      }
    }
  }
}

}; // namespace telemetry
