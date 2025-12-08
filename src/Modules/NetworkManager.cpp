#include "../../include/Modules/NetworkManager.h"
// ==========================================================
// HÀM XỬ LÝ LỆNH TỪ SOCKET
// ==========================================================
void handle_command_from_socket(const std::string& command_line, SOCKET client_socket) {
    // Tương tự như logic trong main() của bạn
    std::stringstream ss(command_line);
    std::string commandName;
    std::string args;

    // 1. Tách tên command (phần đầu tiên)
    ss >> commandName;
    
    // 2. Lấy phần còn lại làm args
    std::getline(ss, args);
    
    // Xóa khoảng trắng đầu (nếu có)
    if (!args.empty() && args[0] == ' ') {
        args = args.substr(1);
    }

    // 3. Gọi Factory và thực thi
    ICommand* cmd = CommandFactory::create(commandName);
    std::string response;
    
    if (cmd) {
        response = cmd->execute(args);
        delete cmd;
    } else {
        response = "[ERROR] Command not found: " + commandName;
    }

    // 4. Gửi phản hồi lại Node.js Backend
    // Thêm ký tự xuống dòng ('\n') để Node.js biết kết thúc tin nhắn
    response += "\n"; 
    send(client_socket, response.c_str(), response.length(), 0);
}


// ==========================================================
// HÀM CHÍNH CHẠY TCP SERVER
// ==========================================================
void start_tcp_server() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return;
    }

    struct addrinfo *result = nullptr, hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(nullptr, DEFAULT_PORT, &hints, &result) != 0) {
        std::cerr << "getaddrinfo failed." << std::endl;
        WSACleanup();
        return;
    }

    SOCKET ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        std::cerr << "socket failed: " << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        WSACleanup();
        return;
    }

    if (bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
        std::cerr << "bind failed: " << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return;
    }
    freeaddrinfo(result);

    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "listen failed: " << WSAGetLastError() << std::endl;
        closesocket(ListenSocket);
        WSACleanup();
        return;
    }
    
    std::cout << "C++ Core Server listening on port " << DEFAULT_PORT << std::endl;
    std::cout << "Waiting for Node.js Backend connection...\n";

    SOCKET ClientSocket = accept(ListenSocket, nullptr, nullptr);
    if (ClientSocket == INVALID_SOCKET) {
        std::cerr << "accept failed: " << WSAGetLastError() << std::endl;
        closesocket(ListenSocket);
        WSACleanup();
        return;
    }
    closesocket(ListenSocket); 
    std::cout << "Node.js Backend connected!\n";

    char recvbuf[BUF_LEN];
    int iResult;

    // Vòng lặp nhận dữ liệu
    do {
        ZeroMemory(recvbuf, BUF_LEN);
        iResult = recv(ClientSocket, recvbuf, BUF_LEN - 1, 0); // Đọc tin nhắn
        
        if (iResult > 0) {
            std::string command(recvbuf, iResult);
            // Xóa ký tự xuống dòng ('\n') mà Node.js gửi kèm
            if (!command.empty() && command.back() == '\n') {
                command.pop_back(); 
            }
            std::cout << "[INFO] Received: " << command << std::endl;
            handle_command_from_socket(command, ClientSocket);
        } else if (iResult == 0) {
            std::cout << "Node.js connection closed. Re-waiting for connection...\n";
            break; // Thoát vòng lặp để chờ kết nối mới
        } else {
            std::cerr << "recv failed: " << WSAGetLastError() << std::endl;
            break;
        }

    } while (iResult > 0);

    closesocket(ClientSocket);
    WSACleanup();

    // Nếu bạn muốn Server C++ luôn chạy và chờ kết nối:
    // start_tcp_server(); // Tự gọi lại để chờ kết nối mới (cần cẩn thận với thread)
}