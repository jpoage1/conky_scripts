#include "log.hpp"

LogLevel g_currentLogLevel = LogLevel::INFO;

const std::array<std::string, static_cast<size_t>(LogLevel::COUNT)>
    LOG_LEVEL_STRINGS = {
        "[DEBUG]: ", "[INFO]: ", "[NOTICE]: ", "[WARNING]: ", "[ERROR]: "};

void log(LogLevel level, std::string log_message) {
  if (level >= g_currentLogLevel) {
    // 3. Perform the direct lookup
    // We cast the enum 'level' to size_t to use it as an array index.
    const std::string& prefix = LOG_LEVEL_STRINGS[static_cast<size_t>(level)];

    std::cerr << prefix << log_message << std::endl;
  }
}
void setLogLevel(LogLevel newLevel) {
  g_currentLogLevel = newLevel;

  // We can even log that the level changed
  if (g_currentLogLevel <= LogLevel::INFO) {
    // Use the array to get the name
    size_t index = static_cast<size_t>(newLevel);
    std::string_view levelName = LogLevelNames[index];

    std::cerr << "[SYSTEM] Log level set to " << levelName << std::endl;
  }
}
