#include "teebuf.hpp"

#include <execinfo.h>

#include <array>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <system_error>
#include <vector>

#include "io.hpp"
namespace telemetry {
void log_backtrace() {
  void *buffer[10];
  int nptrs = backtrace(buffer, 10);
  char **symbols = backtrace_symbols(buffer, nptrs);
  if (symbols == nullptr)
    return;

  std::fprintf(stderr, "  [STACK TRACE]\n");
  // Start at 2 to skip log_backtrace and TeeBuf::sync themselves
  for (int i = 2; i < nptrs; i++) {
    std::fprintf(stderr, "    -> %s\n", symbols[i]);
  }
  free(symbols);
}

TeeBuf::TeeBuf() {
  // Disable buffering: every character goes directly to overflow()
  setp(0, 0);
}
TeeBuf::~TeeBuf() {}

void TeeBuf::add_sink(std::streambuf *s) {
  std::lock_guard<std::mutex> lock(mtx);
  if (s && std::find(sinks.begin(), sinks.end(), s) == sinks.end()) {
    sinks.push_back(s);
  }
}

void TeeBuf::clear_sinks() {
  std::lock_guard<std::mutex> lock(mtx);
  sinks.clear();
}
int TeeBuf::sync() {
  std::lock_guard<std::mutex> lock(mtx);

  if (is_colored || state != TEXT) {
    std::fprintf(
        stderr,
        "\n\033[1;33m[ANSI_NOTICE]\033[0m Unterminated sequence detected!\n"
        "  State: %d (%s)\n"
        "  Colored: %s\n"
        "  Buffer Fragment: '%s'\n",
        (int)state,
        (state == ESCAPED   ? "ESCAPED"
         : state == BRACKET ? "BRACKET"
                            : "TEXT"),
        is_colored ? "YES" : "NO", buffer_fragment.c_str());

    log_backtrace();
  }

  int result = 0;
  for (auto *s : sinks) {
    if (s->pubsync() == -1)
      result = -1;
  }
  return result;
}
std::streamsize TeeBuf::xsputn(const char *s, std::streamsize n) {
  std::lock_guard<std::mutex> lock(mtx);
  for (std::streamsize i = 0; i < n; ++i) {
    update_color_state(s[i]);
    for (auto *sink : sinks) {
      if (sink->sputc(s[i]) == traits_type::eof())
        return i;
    }

    // IMMEDIATE PROTECTION:
    // If we hit a newline while colored, throw BEFORE sync()
    if (s[i] == '\n' && (is_colored || state != TEXT)) {
      throw std::runtime_error(
          "KERNEL_IO_ERROR: Newline detected in unclosed ANSI sequence.");
    }
  }
  return n;
}

TeeBuf::int_type TeeBuf::overflow(int_type c) {
  if (c == traits_type::eof())
    return traits_type::not_eof(c);
  char ch = traits_type::to_char_type(c);

  std::lock_guard<std::mutex> lock(mtx);
  update_color_state(ch);

  for (auto *s : sinks) {
    if (s->sputc(ch) == traits_type::eof())
      return traits_type::eof();
  }

  if (ch == '\n' && (is_colored || state != TEXT)) {
    throw std::runtime_error(
        "KERNEL_IO_ERROR: Newline detected in unclosed ANSI sequence.");
  }

  return c;
}

void TeeBuf::update_color_state(char c) {
  // Use fprintf to stderr to bypass the current buffer logic
  fprintf(stderr, "[DEBUG_TRACE] Char: 0x%02x State: %d\n", (unsigned char)c,
          (int)state);

  switch (state) {
  case TEXT:
    if (c == '\033') {
      state = ESCAPED;
      buffer_fragment = c;
    }
    break;
  case ESCAPED:
    if (c == '[') {
      state = BRACKET;
      buffer_fragment += c;
    } else {
      state = TEXT;
      buffer_fragment.clear();
    }
    break;
  case BRACKET:
    buffer_fragment += c;
    // Verification: Ensure the sequence isn't being prematurely terminated
    if (c >= 0x40 && c <= 0x7E) {
      if (buffer_fragment == "\033[0m") {
        is_colored = false;
      } else if (c == 'm') {
        is_colored = true;
      }
      buffer_fragment.clear();
      state = TEXT;
    }
    break;
  }
}
}; // namespace telemetry
