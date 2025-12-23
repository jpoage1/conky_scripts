// teebuff.hpp
#ifndef TEEBUF_HPP
#define TEEBUF_HPP

#include <algorithm>
#include <chrono>
#include <format>
#include <iostream>
#include <mutex>
#include <string>
#include <vector>

enum AnsiState { TEXT, ESCAPED, BRACKET };

class TeeBuf : public std::streambuf {
  std::vector<std::streambuf*> sinks;
  std::mutex mtx;

  bool is_colored = false;
  std::string buffer_fragment;

  AnsiState state = TEXT;

 public:
  TeeBuf();
  ~TeeBuf();

  void add_sink(std::streambuf* s);

  void clear_sinks();

 protected:
  int_type overflow(int_type c) override;

  int sync() override;

  std::streamsize xsputn(const char* s, std::streamsize n);

 private:
  void update_color_state(char c);
};

void log_backtrace();

#endif
