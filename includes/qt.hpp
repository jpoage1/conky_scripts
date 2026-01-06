// includes/qt.hpp
#ifndef QT_HPP
#define QT_HPP

#include "types.hpp"

class SystemMetricsQtProxy;
struct RunnerContext;

void register_qt_pipeline();
PipelineFactory qt_widget_factory(SystemMetricsQtProxy *proxy);

int qt_main(const RunnerContext &ctx);
#endif
