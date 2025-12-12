// stream_provider.hpp
#pragma once

#include "metrics.hpp"
#include "pcn.hpp"

enum LogLevel {
  Debug,
  Warning,
  Notice,
  None,
};

void dump_fstream(std::istream& stream);

void log_stream_state(const std::istream& stream, const LogLevel log_level,
                      const std::string time, const std::string stream_name);

void log_stream_state(std::istream& stream, const LogLevel log_level,
                      const std::string time);

void log_stream_state(const std::string time, const std::string stream_name,
                      const std::string state);
