#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <vector>
#include "../include/Socket.h"
#include "../include/CommandFactory.h"
#include "../include/ICommand.h"

// Liên kết thư viện Winsock 2.2
#pragma comment(lib, "ws2_32.lib")

// Cổng TCP mà Node.js Proxy sẽ kết nối đến
#define CORE_SERVER_PORT 8888 

// ==================== HÀM HỖ TRỢ GIAO TIẾP ====================

// Gửi phản hồi TEXT TCP thô về Node.js Proxy
void sendResponse(SOCKET client, const std::string& msg)
{
    // Thêm ký tự xuống dòng để Node.js biết khi nào tin nhắn kết thúc
    std::string response = msg + "\n"; 
    send(client, response.c_str(), response.length(), 0);
}

// ==================== HÀM XỬ LÝ LỆNH CHÍNH ====================

// Hàm này sẽ phân tích cú pháp lệnh và thực thi qua CommandFactory
std::string processCommand(const std::string& rawCommand) {
    
    // Loại bỏ ký tự xuống dòng hoặc khoảng trắng dư thừa
    std::string trimmedCommand = rawCommand;
    if (!trimmedCommand.empty() && trimmedCommand.back() == '\n') {
        trimmedCommand.pop_back();
    }
    
    std::stringstream ss(trimmedCommand);
    std::string cmdName;
    ss >> cmdName; // Lấy lệnh chính (ví dụ: "application")

    std::string args;
    if (ss.peek() == ' ') ss.ignore(1);
    std::getline(ss, args); // Lấy đối số (ví dụ: "list" hoặc "start notepad.exe")
    
    // 1. Tạo và thực thi Command
    ICommand* command = CommandFactory::create(cmdName);

    if (command) {
        std::string result = command->execute(args);
        delete command;
        return result;
    } 
    
    // 2. Xử lý lệnh không xác định
    return "[ERROR] Command '" + cmdName + "' not recognized or implemented.";
}


// ==================== SERVER CHÍNH ====================

void startSocketServer()
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return;
    }

    SOCKET server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == INVALID_SOCKET) {
        std::cerr << "Could not create socket.\n";
        WSACleanup();
        return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(CORE_SERVER_PORT); // Lắng nghe trên cổng 8888

    if (bind(server, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed with error: " << WSAGetLastError() << "\n";
        closesocket(server);
        WSACleanup();
        return;
    }

    listen(server, 1);
    std::cout << "C++ Core Server running on port " << CORE_SERVER_PORT << ". Waiting for Node.js Proxy...\n";

    while (true) // Vòng lặp chấp nhận kết nối (chỉ chấp nhận một kết nối tại một thời điểm)
    {
        SOCKET client = accept(server, NULL, NULL);
        if (client == INVALID_SOCKET) {
            std::cerr << "Accept failed: " << WSAGetLastError() << "\n";
            continue;
        }

        std::cout << "Node.js Proxy connected!\n";

        char buffer[2048];
        int len;

        // Vòng lặp nhận lệnh từ Proxy Node.js
        while ((len = recv(client, buffer, sizeof(buffer) - 1, 0)) > 0)
        {
            buffer[len] = 0;
            std::string command = buffer;
            std::cout << "Received: " << command; // In ra lệnh đã nhận (ví dụ: "application list\n")

            // Xử lý và thực thi lệnh
            std::string result = processCommand(command);

            // Gửi phản hồi
            sendResponse(client, result);
        }

        std::cout << "Node.js Proxy disconnected. Cleaning up...\n";
        closesocket(client);
    }

    closesocket(server);
    WSACleanup();
}