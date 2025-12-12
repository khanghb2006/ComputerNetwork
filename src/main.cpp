// main.cpp

#include <iostream>
#include "..\include\Socket.h" 

int main() {
    std::cout << "Initializing C++ Agent...\n";
    // Gọi Socket Server mới lắng nghe TCP trên 8888
    startSocketServer(); 
    return 0;
}

/*
BUILD:
g++ -std=c++17 -I./include src/main.cpp src/Socket.cpp src/core/CommandFactory.cpp src/modules/ApplicationManager.cpp src/Commands/ApplicationCommand.cpp src/modules/Screenshot.cpp src/Commands/ScreenshotCommand.cpp -o Agent.exe -lgdi32 -luser32 -lws2_32
D:\24CNTN_HCMUS\Project\HBK-CNet\backend
*/

// ĐỔI IP máy nạn nhân trong script.js và server.js