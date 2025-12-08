#include <iostream>
#include <sstream>
#include <string>
// Thêm các thư viện Winsock cho TCP/IP
#include <winsock2.h>
#include <ws2tcpip.h>

// #include các file cần thiết của bạn
#include "../include/CommandFactory.h"
#include "../include/ICommand.h" 
#include "../include/Modules/NetworkManager.h"

// ==========================================================
// HÀM MAIN CHÍNH
// ==========================================================
int main() {
    std::cout << "=== C++ Core Server Starting ===\n";
    
    // GỌI HÀM KHỞI ĐỘNG TCP SERVER
    start_tcp_server();

    std::cout << "=== C++ Core Server Stopped ===\n";

    return 0;
}