#pragma once
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#include "pcn.hpp"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

// --- Helper Functions to extract the "Real" Address ---

// 1. For Pointers (T*)
template <typename T>
const void* get_real_addr(T* ptr) {
  return static_cast<const void*>(ptr);
}

// 2. For Smart Pointers (std::unique_ptr, std::shared_ptr)
template <typename T>
const void* get_real_addr(const std::unique_ptr<T>& ptr) {
  return static_cast<const void*>(ptr.get());
}

// 3. For References / Regular Objects (T&)
template <typename T>
const void* get_real_addr(const T& obj) {
  return static_cast<const void*>(&obj);
}

// --- The Macro ---
// Uses SPDLOG_TRACE so it can be compiled out.
// Usage: TRACE_PTR("My Object", my_variable);
#define TRACE_PTR(msg, var) SPDLOG_TRACE("{}: {}", msg, get_real_addr(var))

// Usage: DEBUG_PTR("My Object", my_variable);
// Visible in debug builds if runtime level is set.
#define DEBUG_PTR(msg, var) SPDLOG_DEBUG("{}: {}", msg, get_real_addr(var))

inline void configure_log_level(std::string level) {
  // Normalize to lowercase
  std::transform(level.begin(), level.end(), level.begin(), ::tolower);

  if (level == "trace") {
    spdlog::set_level(spdlog::level::trace);
  } else if (level == "debug") {
    spdlog::set_level(spdlog::level::debug);
  } else if (level == "info") {
    spdlog::set_level(spdlog::level::info);
  } else if (level == "warn" || level == "warning") {
    spdlog::set_level(spdlog::level::warn);
  } else if (level == "err" || level == "error") {
    spdlog::set_level(spdlog::level::err);
  } else if (level == "critical") {
    spdlog::set_level(spdlog::level::critical);
  } else if (level == "off") {
    spdlog::set_level(spdlog::level::off);
  } else {
    // Default safe fallback
    spdlog::set_level(spdlog::level::info);
    SPDLOG_WARN("Unknown log level '{}' in config. Defaulting to INFO.", level);
  }
  std::cerr << "Log level: " << level << std::endl;
}
