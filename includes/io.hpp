// io.hpp
#ifndef IO_HPP
#define IO_HPP

#include <iostream>
#include <memory>
#include <streambuf>

#include "teebuf.hpp"

class RotatingFileSink;

void setup_io(std::streambuf* out_socket = nullptr,
              std::streambuf* err_socket = nullptr);

#endif

extern TeeBuf _kout_buf;
extern TeeBuf _kerr_buf;

// The only two streams you will ever see in your logic
extern std::ostream kout;
extern std::ostream kerr;

extern std::unique_ptr<RotatingFileSink> global_out_file;
extern std::unique_ptr<RotatingFileSink> global_err_file;
