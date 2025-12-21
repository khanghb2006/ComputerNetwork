#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <fstream>
#include "Socket.h"
#include <thread>
#pragma comment(lib, "ws2_32.lib")
#include "CommandFactory.h"
#include "modules/Webcam.h"
#include "SystemControl.h"

const std::string VIDEO_ROOT = "D:/Project_ComputerNetwork/Server/Video/";

// CLIENT HTTP HANDLING

void handleHttpClient(SOCKET client) {
    char buffer[4096]{};
    int len = recv(client, buffer, sizeof(buffer) - 1, 0);
    if (len <= 0) {
        closesocket(client);
        return;
    }

    std::string req(buffer);

    if (req.find("GET /Video/") != 0) {
        closesocket(client);
        return;
    }

    // Parse path
    size_t p1 = req.find("GET ") + 4;
    size_t p2 = req.find(" HTTP/");
    std::string webPath = req.substr(p1, p2 - p1);

    std::string filePath =
        VIDEO_ROOT + webPath.substr(strlen("/Video/"));

    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        closesocket(client);
        return;
    }

    file.seekg(0, std::ios::end);
    long long fileSize = file.tellg();
    file.seekg(0);
    
    std::string header =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: video/mp4\r\n"
        "Content-Length: " + std::to_string(fileSize) + "\r\n"
        "Accept-Ranges: bytes\r\n\r\n";

    send(client, header.c_str(), header.size(), 0);

    char buf[8192];
    while (!file.eof()) {
        file.read(buf, sizeof(buf));
        int n = (int)file.gcount();
        if (n > 0)
            send(client, buf, n, 0);
    }

	closesocket(client);
}


void startHttpServer() {
    SOCKET server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == INVALID_SOCKET) {
        std::cout << "HTTP socket create failed\n";
        return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);

    if (bind(server, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cout << "HTTP bind failed(port 8080)\n";
        closesocket(server);
        return;
    }

    listen(server, SOMAXCONN);
    std::cout << "HTTP Server running on http ://0.0.0.0:8080" << "\n";

    while (true) {
        SOCKET client = accept(server, nullptr, nullptr);
        if (client != INVALID_SOCKET) {
            std::thread(handleHttpClient, client).detach();
        }
    }
}

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    std::thread(startHttpServer).detach();   // HTTP 8080
    std::thread(startSocketServer).detach(); // WS 8081

	// Keep main thread alive
    while (true) {
        Sleep(1000);
    }

    WSACleanup();
    return 0;
}