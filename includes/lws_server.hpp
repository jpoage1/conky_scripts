// includes/lwh_main.hpp
#ifndef LWS_SERVER_HPP
#define LWS_SERVER_HPP

#include <libwebsockets.h>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace libwebsockets {
struct LwsMessage {
  std::vector<uint8_t> buffer;

  explicit LwsMessage(const std::string &data) {
    // LWS requires LWS_PRE bytes of padding before the payload
    buffer.resize(LWS_PRE + data.size());
    std::copy(data.begin(), data.end(), buffer.begin() + LWS_PRE);
  }

  uint8_t *payload() { return buffer.data() + LWS_PRE; }
  size_t length() const { return buffer.size() - LWS_PRE; }
};

class LwsServer {
public:
  LwsServer(int port);
  ~LwsServer();

  void step(); // Single iteration of the event loop
  void broadcast(const std::string &data);

private:
  struct lws_context *context;
  static struct lws_protocols protocols[];

  struct SessionData {
    std::shared_ptr<LwsMessage> current_msg;
  };

  static int callback_telemetry(struct lws *wsi,
                                enum lws_callback_reasons reason, void *user,
                                void *in, size_t len);
};

}; // namespace libwebsockets
#endif
