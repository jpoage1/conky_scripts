#include <istream>

// #include "data.h"
#include "local.h"
// #include "ssh.h"

int main() {
  LocalDataStreams streams = get_local_file_streams();
  SystemMetrics metrics = read_data(streams);
  print_metrics(metrics);
  return 0;
}
