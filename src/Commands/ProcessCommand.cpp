#include "../../include/Commands/ProcessCommand.h"
#include "../../include/Modules/ProcessManager.h"
#include <sstream>
#include <vector>

// Hàm helper để chuyển đổi vector ProcessInfo thành chuỗi
std::string ProcessInfoVectorToString(const std::vector<ProcessInfo>& list) {
    std::stringstream ss;
    ss << "PID\t\tNAME\n";
    ss << "--------------------------------------------------\n";
    for (const auto& info : list) {
        // Đảm bảo truy cập đúng trường dữ liệu của struct ProcessInfo
        ss << info.pid << "\t\t" << info.name << "\n"; 
    }
    return ss.str();
}


std::string ProcessCommand::execute(const std::string& args) {
    std::stringstream ss(args);
    std::string action;
    ss >> action;

    // ---- LIST ----
    if (action == "list") {
        // 1. Gọi module để lấy dữ liệu process
        auto list = ProcessModule::list();

        // 2. Chuyển đổi sang chuỗi và trả về
        if (!list.empty()) {
             std::string process_data = ProcessInfoVectorToString(list);
             return "SUCCESS: Running Processes:\n" + process_data;
        } else {
             return "ERROR: Process list is empty or could not be retrieved.";
        }
    }

    // ---- KILL <PID> ----
    if (action == "kill") {
        unsigned long pid;
        ss >> pid;

        if (!ss) return "ERROR: Invalid PID";

        bool ok = ProcessModule::kill(pid);
        return ok ? "SUCCESS: Process killed" : "ERROR: Process kill failed (Access Denied or PID not found)";
    }

    return "ERROR: Unknown process command: " + args;
}