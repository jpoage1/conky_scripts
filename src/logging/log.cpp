// src/log.cpp
#include "log.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>

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

void configure_log_level(std::string level) {
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
