// Trong NetworkManager.h
#include <iostream>
#include <sstream>
#include <winsock2.h>
#include <ws2tcpip.h>

// #include các file cần thiết của bạn
#include "../../include/CommandFactory.h"
#include "../../include/ICommand.h"

#pragma comment(lib, "ws2_32.lib")

#define DEFAULT_PORT "8888"
#define BUF_LEN 512

void start_tcp_server(); 
void handle_command_from_socket(const std::string& command_line, SOCKET client_socket);