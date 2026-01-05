#include <ixwebsocket/IXWebSocketServer.h>
#include <ixwebsocket/IXWebSocket.h>
#include "json_serializer.hpp"

WebSocketServer::WebSocketServer(int port = 8080) : server(port) {
    server.setOnClientMessageCallback([this](std::shared_ptr<ix::ConnectionState> connectionState, 
                                            ix::WebSocket & webSocket, 
                                            const ix::WebSocketMessagePtr & msg) {
        if (msg->type == ix::WebSocketMessageType::Open) {
            std::lock_guard<std::mutex> lock(clients_mtx);
            clients.insert(&webSocket);
        } else if (msg->type == ix::WebSocketMessageType::Close) {
            std::lock_guard<std::mutex> lock(clients_mtx);
            clients.erase(&webSocket);
        }
    });
}

void WebSocketServer::start() { server.listen(); server.start(); }

void WebSocketServer::broadcast(const nlohmann::json& data) {
    std::string payload = data.dump();
    std::lock_guard<std::mutex> lock(clients_mtx);
    for (auto client : clients) {
        client->send(payload);
    }
}
