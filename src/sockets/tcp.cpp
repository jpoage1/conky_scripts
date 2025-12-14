#include <arpa/inet.h>
#include <sys/socket.h>

void send_json_over_tcp(const nlohmann::json& j, const std::string& server_ip,
                        int port) {
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in serv_addr;
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);
  inet_pton(AF_INET, server_ip.c_str(), &serv_addr.sin_addr);

  if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
    SPDLOG_ERROR("Connection Failed");
    return;
  }

  std::string payload = j.dump() + "\n";  // Add newline delimiter
  send(sock, payload.c_str(), payload.length(), 0);
  close(sock);
}
