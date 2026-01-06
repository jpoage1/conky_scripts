// src/libwebsockets/lwh_main.cpp
#include "controller.hpp"
#include "json_serializer.hpp"
#include "libwebsockets.hpp"
#include "lws_proxy.hpp"
#include "lws_server.hpp"
#include "runner_context.hpp"
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

namespace libwebsockets {

int lws_main(const RunnerContext &ctx) {
  LwsServer server(8080);

  // Cast the base proxy back to our LWS-specific version
  auto proxy = std::dynamic_pointer_cast<SystemMetricsLwsProxy>(
      ctx.controller->get_proxy());

  while (ctx.controller->is_persistent()) {
    server.step();          // Process LWS network events
    ctx.controller->tick(); // Run telemetry tasks -> updates Proxy

    if (proxy && proxy->ready()) {
      server.broadcast(proxy->consume());
    }

    ctx.controller->sleep(); // Frequency control
  }
  return 0;
}
}; // namespace libwebsockets
