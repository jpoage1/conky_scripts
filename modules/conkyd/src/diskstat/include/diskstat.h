
#include <vector>

#include "colors.h"
#include "conky_format.h"
#include "device_info.h"
#include "print_utils.h"
#include "size_format.h"
#include "types.h"

static std::vector<std::string> read_device_paths(const std::string &path);

int diskstat(const std::string &config_file);
