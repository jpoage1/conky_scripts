// include/load_avg.hpp
#ifndef LOAD_AVG
#define LOAD_AVG

#include "pcn.hpp"

class SystemMetrics;

void get_load_and_process_stats(std::istream& stream, SystemMetrics& metrics);

#endif
