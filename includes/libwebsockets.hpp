// includes/lwh_main.hpp
#ifndef LIBWEBSOCKETS_HPP
#define LIBWEBSOCKETS_HPP

#include "types.hpp"

struct RunnerContext;
namespace libwebsockets {
void register_libwebsockets_pipeline();
telemetry::PipelineFactory libwebsockets_stream();

int lws_main(const telemetry::RunnerContext &ctx);

}; // namespace libwebsockets
#endif
