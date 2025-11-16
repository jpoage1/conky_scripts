// meminfo.h

#pragma once
#include "pcn.hpp"

void get_mem_usage(std::istream&, long&, long&, int&);

void get_swap_usage(std::istream&, long&, long&, int&);
