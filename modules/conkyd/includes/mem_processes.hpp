// mem_processes.hpp
#pragma once

#include "data_local.h"
#include "data_ssh.h"

void get_top_processes_mem(std::istream& stream, SystemMetrics& metrics);
