#ifndef SSH_H
#define SSH_H

#include <string>

int ssh_connection();
int setup_ssh_session();
std::string execute_ssh_command(const std::string&);
void cleanup_ssh_session();

#endif  // SSH_H
