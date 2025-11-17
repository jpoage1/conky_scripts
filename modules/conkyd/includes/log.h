#ifndef LOG_H
#define LOG_H

// This file provides a simple preprocessor-based logger.
// It checks for the DEBUG_LOG flag (compile-time) and a runtime log level.

#include <iostream>

// --- Log Level Definitions ---
enum class LogLevel {
  DEBUG = 10,
  NOTICE = 20,
  WARNING = 30,
  ERROR = 40,
  NONE = 100  // Level to disable all logging
};

// --- Runtime Log Level Management (Header-only) ---

// Holds the current log level.
// We use a static function variable to keep it contained in this header
// and avoid needing a separate .cpp file for definition.
inline LogLevel& current_log_level() {
  // Default level is NOTICE.
  // Can be changed by calling set_log_level().
  static LogLevel level = LogLevel::NOTICE;
  return level;
}

// Public function to set the log level
inline void set_log_level(LogLevel level) { current_log_level() = level; }

// --- Log Macros ---
// We use do-while(0) to make the macros safe statements
// (e.g., inside an `if` without braces)

// --- Level 1: Debug ---
// Verbose, development-only messages.
// Requires *both* the compile-time flag AND the runtime level.
#if DEBUG_LOG
#define LOG_DEBUG(msg)                                                       \
  do {                                                                       \
    if (current_log_level() <= LogLevel::DEBUG) {                            \
      std::cerr << "[DEBUG] (" << __FILE__ << ":" << __LINE__ << ") " << msg \
                << std::endl;                                                \
    }                                                                        \
  } while (0)
#else
#define LOG_DEBUG(msg) \
  do {                 \
  } while (0)  // Compiled out
#endif

// --- Level 2: Notice ---
// Standard informational messages.
#define LOG_NOTICE(msg)                             \
  do {                                              \
    if (current_log_level() <= LogLevel::NOTICE) {  \
      std::cout << "[NOTICE] " << msg << std::endl; \
    }                                               \
  } while (0)

// --- Level 3: Warning ---
// Important messages about potential issues.
#define LOG_WARNING(msg)                                                    \
  do {                                                                      \
    if (current_log_level() <= LogLevel::WARNING) {                         \
      std::cerr << "[WARN] (" << __FILE__ << ":" << __LINE__ << ") " << msg \
                << std::endl;                                               \
    }                                                                       \
  } while (0)

// --- Level 4: Error ---
// Critical, non-recoverable issues.
#define LOG_ERROR(msg)                                                       \
  do {                                                                       \
    if (current_log_level() <= LogLevel::ERROR) {                            \
      std::cerr << "[ERROR] (" << __FILE__ << ":" << __LINE__ << ") " << msg \
                << std::endl;                                                \
    }                                                                        \
  } while (0)

#endif  // LOG_H
