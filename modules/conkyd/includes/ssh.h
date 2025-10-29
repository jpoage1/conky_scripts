// ssh.h
#ifndef SSH_H
#define SSH_H

#include <string>

int setup_ssh_session();
int setup_ssh_session(const std::string host, const std::string user);

int ssh_connection();
int ssh_connection(const std::string host, const std::string user);

std::string execute_ssh_command(const std::string&);
void cleanup_ssh_session();

void generate_local_metrics(const std::string& config_file);
void generate_server_metrics(const std::string& config_file);
void generate_server_metrics(const std::string& config_file,
                             const std::string& host, const std::string& user);

#endif  // SSH_H
