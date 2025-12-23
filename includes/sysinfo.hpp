// sysinfo.hpp
#ifndef SYSINFO_HPP
#define SYSINFO_HPP
#include <sys/utsname.h>

class SystemMetrics;

void get_system_info(SystemMetrics& metrics);

#endif
