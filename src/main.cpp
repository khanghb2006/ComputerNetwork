#include <iostream>
#include <string>
#include <memory> // Vẫn giữ unique_ptr nhưng không dùng cho Application nữa.
#include <sstream> // Cần để parse input

// Include Factory và Interface
#include "../include/CommandFactory.h"
// Bỏ include "../include/modules/Application.h"
// Bỏ include platform-specific implementation: Application_Windows.h
// Logic platform-specific đã nằm trong ApplicationManager.cpp

// --- Helper: Factory function to create Application based on OS ---
// Hàm này không còn cần thiết nữa. Xóa hoặc ghi chú.
/*
std::unique_ptr<Application> createApplication() {
#ifdef _WIN32
    return std::make_unique<Application_Windows>(); 
#else
    std::cerr << "Error: This platform is not supported yet.\n";
    return nullptr;
#endif
}
*/

int main() {
    // 1. Initialize the application
    // Bỏ việc tạo đối tượng Application
    // auto app = createApplication(); 
    // if (!app) { return 1; }

    std::string inputLine;

    // Print usage instructions (Đã thay đổi để phù hợp với lệnh chung "application")
    std::cout << "=== APPLICATION MANAGER CLI ===\n";
    std::cout << "Available commands (Prefix: application):\n";
    std::cout << " 1. application list              : List all running applications into file text (runningApp.txt)\n";
    std::cout << " 2. application start [path]      : Start an application (e.g., application start notepad.exe)\n";
    std::cout << " 3. application stop [name|path]  : Stop an application by name/path (e.g., application stop notepad.exe)\n";
    std::cout << " 4. screenshot [path]             : Take a screenshot (e.g., screenshot myimage.bmp)\n";
    std::cout << " 5. exit                          : Exit the program\n";
    std::cout << "===============================\n";

    // 2. Main loop
    while (true) {
        std::cout << "\n> "; // Command prompt
        
        if (!std::getline(std::cin, inputLine)) {
            break; 
        }

        if (inputLine.empty()) continue;
        if (inputLine == "exit") break;

        // 3. Parse Command Name and Arguments
        // Logic mới: Tách lệnh chính (cmdName) và phần còn lại (args)
        // Ví dụ 1: "application start notepad.exe" -> cmdName="application", args="start notepad.exe"
        // Ví dụ 2: "screenshot myimg.bmp"        -> cmdName="screenshot", args="myimg.bmp"
        
        std::string cmdName;
        std::string args;
        
        std::stringstream ss(inputLine);
        ss >> cmdName; // Lấy từ đầu tiên làm cmdName

        // Lấy phần còn lại của chuỗi (bao gồm cả khoảng trắng đầu)
        if (ss.peek() == ' ') ss.ignore(1);
        std::getline(ss, args);

        // 4. Use Factory to create Command object
        // Bỏ tham số app.get()
        ICommand* command = CommandFactory::create(cmdName); 

        if (command != nullptr) {
            // 5. Execute command and print result
            std::string result = command->execute(args);
            std::cout << result << std::endl;

            // 6. Memory cleanup (IMPORTANT)
            delete command;
        } else {
            std::cout << "Error: Unknown command '" << cmdName << "'. Type 'exit' to quit.\n";
        }
    }

    std::cout << "Program exited.\n";
    return 0;
}

/* Build file:
g++ -std=c++17 -I./include src/main.cpp src/core/CommandFactory.cpp src/modules/ApplicationManager.cpp src/modules/Screenshot.cpp src/Commands/ApplicationCommand.cpp src/Commands/ScreenshotCommand.cpp -o AppManager.exe -lgdi32 -luser32
*/