// cpu_processes.hpp
#pragma once

#include "data_local.h"
#include "data_ssh.h"

void get_top_processes_cpu(DataStreamProvider& provider,
                           SystemMetrics& metrics);
