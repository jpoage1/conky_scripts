// src/controller.hpp
#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include <list>
#include <memory>

// Forward Declarations
class ParsedConfig;
struct MetricsContext;
struct SystemMetrics;

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

 private:
  std::unique_ptr<ParsedConfig> m_config;

  // Note: std::list requires the type to be complete in some implementations.
  // If your compiler complains, change this to
  // std::list<std::unique_ptr<SystemMetrics>>.
  struct SystemMetricsImpl;
  std::unique_ptr<SystemMetricsImpl> tasks_pimpl;
};

#endif
