#ifndef SYSTEM_STABILITY_HPP
#define SYSTEM_STABILITY_HPP
double parse_avg10(const std::string &line);
struct SystemStability {
  long file_descriptors_allocated = 0;
  long file_descriptors_max = 0;
  // Pressure Stall Information (PSI) - The "Lag" indicator
  double memory_pressure_some = 0.0;
  double cpu_pressure_some = 0.0;
  double memory_pressure_full = 0.0;
  double io_pressure_some = 0.0;
  double io_pressure_full = 0.0;
  double memory_fragmentation_index = 0.0;
};
#endif
