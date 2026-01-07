// include/log.hpp
#ifndef LOG_HPP
#define LOG_HPP
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#include "pcn.hpp"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace telemetry {
// --- Helper Functions to extract the "Real" Address ---

// 1. For Pointers (T*)
template <typename T> const void *get_real_addr(T *ptr);

// 2. For Smart Pointers (std::unique_ptr, std::shared_ptr)
template <typename T> const void *get_real_addr(const std::unique_ptr<T> &ptr);

// 3. For References / Regular Objects (T&)
template <typename T> const void *get_real_addr(const T &obj);

// --- The Macro ---
// Uses SPDLOG_TRACE so it can be compiled out.
// Usage: TRACE_PTR("My Object", my_variable);
#define TRACE_PTR(msg, var) SPDLOG_TRACE("{}: {}", msg, get_real_addr(var))

// Usage: DEBUG_PTR("My Object", my_variable);
// Visible in debug builds if runtime level is set.
#define DEBUG_PTR(msg, var) SPDLOG_TRACE("{}: {}", msg, get_real_addr(var))

void configure_log_level(std::string level);

}; // namespace telemetry
#endif
