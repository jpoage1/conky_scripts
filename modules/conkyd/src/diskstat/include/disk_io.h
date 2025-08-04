#pragma once

#include <string>

std::string get_read_bytes_per_sec(const std::string &device_path);
std::string get_write_bytes_per_sec(const std::string &device_path);

