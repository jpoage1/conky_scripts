#include <istream>

// #include "data.h"
#include "data_local.h"
#include "data_ssh.h"
void get_metrics(LocalDataStreams &streams);
void get_metrics(ProcDataStreams &streams);

int main() {
  LocalDataStreams local_streams = get_local_file_streams();
  get_metrics(local_streams);

  std::cout << "-------------" << std::endl;

  ProcDataStreams ssh_streams = get_ssh_streams();
  get_metrics(ssh_streams);
  return 0;
}

void get_metrics(LocalDataStreams &streams) {
  SystemMetrics metrics = read_data(streams);
  print_metrics(metrics);
}

void get_metrics(ProcDataStreams &streams) {
  SystemMetrics metrics = read_data(streams);
  print_metrics(metrics);
}
