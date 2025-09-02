#pragma once

#include <iostream>
#include <string>

std::string get_mount_point(const std::string &device_path);
std::string get_mount_point(std::istream &, const std::string &device_path);
