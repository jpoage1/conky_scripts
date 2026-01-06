// src/libwebsockets/lwh_server.cpp
#include "lws_server.hpp"
#include <iostream>

namespace libwebsockets {
struct ServerInternalState {
  std::set<struct lws *> clients;
  std::shared_ptr<LwsMessage> pending_broadcast;
};

int LwsServer::callback_telemetry(struct lws *wsi,
                                  enum lws_callback_reasons reason, void *user,
                                  void *in, size_t len) {
  // Explicitly suppress unused parameter warnings for high-signal builds
  (void)user;
  (void)in;
  (void)len;

  auto *state = static_cast<ServerInternalState *>(
      lws_context_user(lws_get_context(wsi)));

  switch (reason) {
  case LWS_CALLBACK_ESTABLISHED:
    state->clients.insert(wsi);
    break;

  case LWS_CALLBACK_CLOSED:
    state->clients.erase(wsi);
    break;

  case LWS_CALLBACK_SERVER_WRITEABLE:
    if (state->pending_broadcast) {
      // Return of lws_write should be checked in production for flow control
      lws_write(wsi, state->pending_broadcast->payload(),
                state->pending_broadcast->length(), LWS_WRITE_TEXT);
    }
    break;

  default:
    break;
  }
  return 0;
}

/**
 * libwebsockets protocol list initialization.
 * Designated initializers ensure all fields are zeroed, satisfying
 * -Wmissing-field-initializers.
 */
struct lws_protocols LwsServer::protocols[] = {
    {
        "telemetry-protocol",          // name
        LwsServer::callback_telemetry, // callback
        0,                             // per_session_data_size
        4096,                          // rx_buffer_size
        0,                             // id
        nullptr,                       // user
        0                              // tx_packet_size
    },
    {nullptr, nullptr, 0, 0, 0, nullptr, 0} // Sentinel/Termination entry
};
LwsServer::LwsServer(int port) {
  struct lws_context_creation_info info;
  memset(&info, 0, sizeof(info));

  info.port = port;
  info.protocols = protocols;
  info.user = new ServerInternalState(); // Attached to context

  context = lws_create_context(&info);
}
LwsServer::~LwsServer() {
  if (context) {
    lws_context_destroy(context);
  }
}

void LwsServer::broadcast(const std::string &data) {
  auto *state = static_cast<ServerInternalState *>(lws_context_user(context));
  state->pending_broadcast = std::make_shared<LwsMessage>(data);
  lws_callback_on_writable_all_protocol(context, &protocols[0]);
}

void LwsServer::step() { lws_service(context, 0); }
}; // namespace libwebsockets
