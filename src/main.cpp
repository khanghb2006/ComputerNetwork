// #include <iostream>
// #include "../include/CommandFactory.h"

// int main() {
//     std::cout << "=== Command Test Program ===\n";
//     std::cout << "Commands available:\n";
//     std::cout << "  keycatch     - start key catching module\n";
//     std::cout << "  processes    - list running processes\n";
//     std::cout << "----------------------------------------\n";

//     std::string commandName;
//     std::cout << "Enter command: ";
//     std::cin >> commandName;

//     ICommand* cmd = CommandFactory::create(commandName);
//     if (!cmd) {
//         std::cout << "Command not found\n";
//         return 0;
//     }

//     std::string res = cmd->execute("");
//     std::cout << "\n[RESULT]\n" << res << "\n";

//     delete cmd;
//     return 0;
// }

#include <iostream>
#include <sstream> // Thêm thư viện này để tách command và args
#include <limits>  // Thêm thư viện này để xóa bộ đệm (flush buffer)
#include "../include/CommandFactory.h"
#include "../include/ICommand.h" // Thêm include cho ICommand

int main() {
    std::cout << "=== Command Test Program ===\n";
    std::cout << "Usage: <command> [arguments]\n";
    std::cout << "Available commands:\n";
    std::cout << "  process [list|start <path>|kill <pid>]\n";
    std::cout << "  key     [start|stop|dump]\n";
    std::cout << "  exit    (to quit)\n";
    std::cout << "----------------------------------------\n";

    std::string fullInput;
    
    while (true) {
        std::cout << "\nEnter command: ";
        // Đọc toàn bộ dòng nhập liệu
        std::getline(std::cin, fullInput);

        if (fullInput.empty()) {
            continue;
        }
        
        if (fullInput == "exit") {
            break;
        }

        std::stringstream ss(fullInput);
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

        // Tạo Command
        ICommand* cmd = CommandFactory::create(commandName);
        if (!cmd) {
            std::cout << "Command not found: " << commandName << "\n";
            continue;
        }

        // Thực thi Command
        std::string res = cmd->execute(args);
        std::cout << "\n[RESULT]\n" << res << "\n";

        delete cmd;
    }

    return 0;
}
