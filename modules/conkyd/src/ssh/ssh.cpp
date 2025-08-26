#include "ssh.h"

#include <libssh/libssh.h>

#include <iostream>
#include <sstream>

static ssh_session session = NULL;

int setup_ssh_session() {
  if (session != NULL) {
    return 0;  // Session already exists
  }

  session = ssh_new();
  if (session == NULL) {
    std::cerr << "Failed to create SSH session." << std::endl;
    return 1;
  }

  ssh_options_set(session, SSH_OPTIONS_HOST, "192.168.1.200");
  ssh_options_set(session, SSH_OPTIONS_USER, "conky");

  int rc = ssh_connect(session);
  if (rc != SSH_OK) {
    std::cerr << "Error connecting: " << ssh_get_error(session) << std::endl;
    ssh_free(session);
    session = NULL;
    return 1;
  }

  rc = ssh_userauth_publickey_auto(session, NULL, NULL);
  if (rc != SSH_OK) {
    std::cerr << "Public key authentication failed: " << ssh_get_error(session)
              << std::endl;
    ssh_disconnect(session);
    ssh_free(session);
    session = NULL;
    return 1;
  }
  std::cout << "Authentication successful!" << std::endl;
  return 0;
}

std::string execute_ssh_command(const std::string& command) {
  if (session == NULL) {
    return "";  // Session not initialized
  }

  ssh_channel channel = ssh_channel_new(session);
  if (channel == NULL) {
    return "";
  }
  int rc = ssh_channel_open_session(channel);
  if (rc != SSH_OK) {
    ssh_channel_free(channel);
    return "";
  }
  rc = ssh_channel_request_exec(channel, command.c_str());
  if (rc != SSH_OK) {
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    return "";
  }

  char buffer[1024];
  int nbytes;
  std::stringstream remote_output;
  while ((nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0)) > 0) {
    remote_output.write(buffer, nbytes);
  }

  ssh_channel_close(channel);
  ssh_channel_free(channel);
  return remote_output.str();
}

void cleanup_ssh_session() {
  if (session != NULL) {
    ssh_disconnect(session);
    ssh_free(session);
    session = NULL;
  }
}
int ssh_connection() {
  ssh_session session;
  session = ssh_new();
  if (session == NULL) {
    std::cerr << "Failed to create SSH session." << std::endl;
    return 1;
  }

  // Replace with your remote host and username
  ssh_options_set(session, SSH_OPTIONS_HOST, "192.168.1.200");
  ssh_options_set(session, SSH_OPTIONS_USER, "conky");

  // Establish the connection
  int rc = ssh_connect(session);
  if (rc != SSH_OK) {
    std::cerr << "Error connecting: " << ssh_get_error(session) << std::endl;
    ssh_free(session);
    return 1;
  }

  // Authenticate with public key
  rc = ssh_userauth_publickey_auto(session, NULL, NULL);
  if (rc != SSH_OK) {
    std::cerr << "Public key authentication failed: " << ssh_get_error(session)
              << std::endl;
    ssh_disconnect(session);
    ssh_free(session);
    return 1;
  }
  std::cout << "Authentication successful!" << std::endl;

  // Execute a command
  ssh_channel channel = ssh_channel_new(session);
  if (channel == NULL) {
    std::cerr << "Failed to create channel." << std::endl;
    ssh_disconnect(session);
    ssh_free(session);
    return 1;
  }

  rc = ssh_channel_open_session(channel);
  if (rc != SSH_OK) {
    std::cerr << "Failed to open session." << std::endl;
    ssh_channel_free(channel);
    ssh_disconnect(session);
    ssh_free(session);
    return 1;
  }

  rc = ssh_channel_request_exec(channel, "ls -l");  // Command to execute
  if (rc != SSH_OK) {
    std::cerr << "Failed to execute command." << std::endl;
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    ssh_disconnect(session);
    ssh_free(session);
    return 1;
  }

  char buffer[256];
  int nbytes;

  while ((nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0)) > 0) {
    std::cout.write(buffer, nbytes);
  }

  // Clean up
  ssh_channel_close(channel);
  ssh_channel_free(channel);
  ssh_disconnect(session);
  ssh_free(session);

  return 0;
}
