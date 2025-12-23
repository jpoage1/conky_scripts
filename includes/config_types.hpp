// cli_parser.hpp
#ifndef CONFIG_TYPES_HPP
#define CONFIG_TYPES_HPP

#include "context.hpp"
#include "lua_parser.hpp"
#include "metrics.hpp"
#include "pcn.hpp"
#include "types.hpp"

class ParsedConfig {
 private:
  std::chrono::nanoseconds polling_interval = std::chrono::milliseconds(500);
  OutputMode _output_mode = OutputMode::JSON;
  RunMode _run_mode = RunMode::RUN_ONCE;
  OutputPipeline active_pipeline;
  std::chrono::time_point<std::chrono::steady_clock> sleep_until;
  std::string config_path;
  std::filesystem::file_time_type last_write_time;

 public:
  std::vector<MetricsContext> tasks;

  /**
   * @brief A single template setter.
   * It accepts any std::chrono duration and converts it to nanoseconds.
   */
  template <typename DurationType>
  void set_polling_interval(const DurationType& interval) {
    polling_interval =
        std::chrono::duration_cast<std::chrono::nanoseconds>(interval);
  }

  /**
   * @brief A single template getter.
   * It returns the interval cast to whatever type the caller asks for.
   */
  template <typename DurationType>
  DurationType get_polling_interval() const {
    return std::chrono::duration_cast<DurationType>(polling_interval);
  }

  bool run_mode(RunMode mode) const;
  bool output_mode(OutputMode mode) const;
  RunMode run_mode() const;
  OutputMode get_output_mode() const;
  void set_run_mode(RunMode mode);
  void set_output_mode(OutputMode mode);
  void set_output_mode(std::string mode);
  void set_run_mode(std::string mode);
  void configure_renderer();
  void sleep();

  int initialize(std::list<SystemMetrics>& tasks);
  void done(std::list<SystemMetrics>& result);
  bool reload_if_changed(std::list<SystemMetrics>& tasks);
  void set_filename(std::string filename);
};

#endif
