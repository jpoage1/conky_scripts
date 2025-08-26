#include <istream>

// #include "data.h"
#include "data_local.h"
#include "data_ssh.h"
void get_metrics(LocalDataStreams &streams);
void get_metrics(ProcDataStreams &streams);
int get_ssh_metrics();

int main() {
  LocalDataStreams local_streams = get_local_file_streams();
  get_metrics(local_streams);

  std::cout << "-------------" << std::endl;

  //   ProcDataStreams ssh_streams = get_ssh_streams();
  //   get_metrics(ssh_streams);
  get_ssh_metrics();
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
int get_ssh_metrics() {
  DataStreamProvider *provider = nullptr;

  if (setup_ssh_session() != 0) {
    std::cerr << "Failed to set up SSH session. Exiting." << std::endl;
    return 1;
  }
  ProcDataStreams ssh_streams = get_ssh_streams();
  provider = &ssh_streams;

  if (provider) {
    SystemMetrics metrics = read_data(*provider);
    print_metrics(metrics);
  } else {
    std::cerr << "Error: No data provider could be initialized." << std::endl;
    return 1;
  }
  cleanup_ssh_session();
  return 0;
}
