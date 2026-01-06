// src/controller.hpp
#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include <list>
#include <memory>

#include "types.hpp"

// Forward Declarations
class ParsedConfig;
struct MetricsContext;
struct SystemMetrics;
struct RunnerContext;

class Controller {
 public:
  // Constructor and Destructor must be declared here
  // but defined in the .cpp to support unique_ptr with forward declarations.
  Controller();
  ~Controller();

  void initialize(ParsedConfig& config);
  void tick();

  bool is_persistent() const;
  void sleep();

  void inject_task(MetricsContext&& context);

  int main(const RunnerContext& context);
  SystemMetricsProxyPtr get_proxy();

 private:
  std::unique_ptr<ParsedConfig> m_config;

  // Note: std::list requires the type to be complete in some implementations.
  // If your compiler complains, change this to
  // std::list<std::unique_ptr<SystemMetrics>>.
  struct SystemMetricsImpl;
  std::unique_ptr<SystemMetricsImpl> tasks_pimpl;
};

#endif
