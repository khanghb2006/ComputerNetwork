#include <iostream>
#include <sstream>
#include <winsock2.h>
#include <ws2tcpip.h>

// include header files
#include "CommandFactory.h"
#include "ICommand.h"

#pragma comment(lib, "ws2_32.lib")

#define DEFAULT_PORT "8081"
#define BUF_LEN 512

void start_tcp_server();
void handle_command_from_socket(const std::string& command_line, SOCKET client_socket);