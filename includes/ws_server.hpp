#include <ixwebsocket/IXWebSocketServer.h>
#include <ixwebsocket/IXWebSocket.h>
#include "json_serializer.hpp"

class WebSocketServer {
public:
    TelemetryServer(int port = 8080) ;

    void start();

    void broadcast(const nlohmann::json& data);

private:
    ix::WebSocketServer server;
    std::set<ix::WebSocket*> clients;
    std::mutex clients_mtx;
};
