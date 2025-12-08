// ApplicationCommand.cpp
// Giả định file này nằm trong src/Commands/
#include "../../include/Commands/ApplicationCommand.h" 
#include <sstream>
#include <fstream> // Để sử dụng ofstream

std::string ApplicationCommand::execute(const std::string& args) {
    if (args.empty()) {
        return "[Application] Missing command action (list, start, stop).";
    }

    std::stringstream ss(args);
    std::string action;
    ss >> action; // Lấy từ đầu tiên làm action

    // --- 1. LIST APPLICATION ---
    if (action == "list") {
        // Tái tạo logic từ ListApplicationCommand.cpp và hình ảnh ProcessCommand.
        
        // Sử dụng hàm exportApplicationToFile để lưu file.
        std::string defaultFile = "runningApp.txt";
        bool success = ApplicationManager::exportApplicationToFile(defaultFile); // Gọi hàm tĩnh
        
        if (success) {
            return "[Application] Success: List of running applications has been saved to '" + defaultFile + "'.";
        } else {
            return "[Application] Failed: Failed to save application list to '" + defaultFile + "'.";
        }
    }

    // --- 2. START APPLICATION ---
    if (action == "start") {
        std::string path;
        // Lấy phần còn lại của chuỗi, bỏ qua khoảng trắng đầu
        if (ss.peek() == ' ') ss.ignore(1);
        std::getline(ss, path); 

        if (path.empty()) {
            return "[Application] Missing file path to start.";
        }

        // Tái tạo logic từ StartApplicationCommand.cpp
        // startApplication(const std::string& application, const std::string& args = "")
        // Ở đây, chúng ta chỉ truyền path và args rỗng (nếu args không được truyền vào theo format start <path> [args])
        
        auto pidOpt = ApplicationManager::startApplication(path); // Gọi hàm tĩnh
        
        if (pidOpt) {
            return "[Application] Success: Started application with PID: " + std::to_string(*pidOpt);
        } else {
            return "[Application] Failed: Could not start application: " + path;
        }
    }

    // --- 3. STOP APPLICATION ---
    if (action == "stop") {
        std::string appNameOrPath;
        // Lấy phần còn lại của chuỗi, bỏ qua khoảng trắng đầu
        if (ss.peek() == ' ') ss.ignore(1);
        std::getline(ss, appNameOrPath); 
        
        if (appNameOrPath.empty()) {
            return "[Application] Error: Please specify the application name or path to stop.";
        }

        // Tái tạo logic từ StopApplicationCommand.cpp
        bool success = ApplicationManager::stopApplication(appNameOrPath); // Gọi hàm tĩnh
        
        if (success) {
            return "[Application] Success: Application '" + appNameOrPath + "' has been stopped.";
        } else {
            return "[Application] Error: Could not stop application '" + appNameOrPath + "'. It may not be running or Access is Denied.";
        }
    }

    // --- 4. UNKNOWN COMMAND ---
    return "[Application] Unknown application command action: " + action;
}