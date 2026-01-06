#include "types.hpp"

class SystemMetricsQtProxy;
struct RunnerContext;

PipelineFactory qt_widget_factory(SystemMetricsQtProxy* proxy);

int qt_main(const RunnerContext& ctx);
