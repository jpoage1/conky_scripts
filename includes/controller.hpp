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
  virtual ~Controller();

  virtual void initialize(ParsedConfig &config);
  virtual void tick();

  virtual bool is_persistent() const;
  virtual void sleep();

  void inject_task(MetricsContext &&context);

  virtual int main(const RunnerContext &context);
  virtual SystemMetricsProxyPtr get_proxy();

protected:
  std::unique_ptr<ParsedConfig> m_config;

  // Note: std::list requires the type to be complete in some implementations.
  // If your compiler complains, change this to
  // std::list<std::unique_ptr<SystemMetrics>>.
  struct SystemMetricsImpl;
  std::unique_ptr<SystemMetricsImpl> tasks_pimpl;
};

#endif
