// includes/lwh_main.hpp
#ifndef LIBWEBSOCKETS_HPP
#define LIBWEBSOCKETS_HPP

#include "types.hpp"

struct RunnerContext;
namespace libwebsockets {
void register_libwebsockets_pipeline();
PipelineFactory libwebsockets_stream();

int lws_main(const RunnerContext &ctx);
}; // namespace libwebsockets
#endif
