
#include <vector>

#include "colors.h"
#include "conky_format.h"
#include "data.h"
#include "device_info.h"
#include "print_utils.h"
#include "size_format.h"
#include "types.h"

int read_device_paths(const std::string &path, std::vector<std::string> &);

void diskstat(const std::string &config_file);

int diskstat(DataStreamProvider &provider, const std::string &config_file);
