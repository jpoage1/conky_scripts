// cpuinfo.hp
#ifndef CPUINFO_HPP
#define CPUINFO_HPP

#include "pcn.hpp"

namespace telemetry {

float get_cpu_freq_mhz(std::istream &);
float get_cpu_freq_ghz(std::istream &);

}; // namespace telemetry
#endif
