// sysinfo.hpp
#ifndef SYSINFO_HPP
#define SYSINFO_HPP

#include <sys/utsname.h>

namespace telemetry {

class SystemMetrics;

void get_system_info(SystemMetrics &metrics);

}; // namespace telemetry
   //
#endif
