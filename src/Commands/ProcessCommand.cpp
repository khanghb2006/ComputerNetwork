// // src/Commands/ProcessCommand.cpp

// #include "../../include/Commands/ProcessCommand.h"
// #include "../../include/Modules/ProcessManager.h"
// #include <sstream>
// // Bỏ include <fstream> vì không dùng file nữa

// // HÀM CHUYỂN ĐỔI LIST SANG CHUỖI (Cần ProcessManager.h để biết ProcessInfo)
// // Tôi sẽ giả định hàm này được khai báo ở đâu đó, hoặc bạn đặt nó trong file này
// std::string ProcessInfoVectorToString(const std::vector<ProcessInfo>& list) {
//     std::stringstream ss;
//     ss << "PID\t\tNAME\n";
//     ss << "--------------------------------------------------\n";
//     for (const auto& info : list) {
//         ss << info.pid << "\t\t" << info.name << "\n";
//     }
//     return ss.str();
// }

// std::string ProcessCommand::execute(const std::string& args) {

//     std::stringstream ss(args);
//     std::string action;
//     ss >> action;

//     // ---- LIST ----
//     if (action == "list") {
//         // 1. Gọi module để lấy dữ liệu process (std::vector<ProcessInfo>)
//         auto list = ProcessModule::list();

//         // --- LOGIC GHI FILE ĐÃ BỊ LOẠI BỎ ---
//         /*
//         std::ofstream f("process_list.txt");
//         for (auto& p : list) {
//             f << p.pid << " - " << p.name << "\n";
//         }
//         f.close();
//         */

//         // 2. Chuyển đổi sang chuỗi và trả về
//         if (!list.empty()) {
//              std::string process_data = ProcessInfoVectorToString(list);
//              return "SUCCESS: Running Processes:\n" + process_data;
//         } else {
//              return "ERROR: Process list is empty or could not be retrieved.";
//         }
//     }

//     // ---- KILL <PID> ---- (Giữ nguyên)
//     if (action == "kill") {
//         // ... (Logic cũ giữ nguyên) ...
//         unsigned long pid;
//         ss >> pid;

//         if (!ss) return "ERROR: Invalid PID";

//         bool ok = ProcessModule::kill(pid);
//         return ok ? "SUCCESS: Process killed" : "ERROR: Process kill failed (Access Denied or PID not found)";
//     }

//     // ---- START <path> ---- (Giữ nguyên)
//     if (action == "start") {
//         // ... (Logic cũ giữ nguyên) ...
//         std::string path;
//         std::getline(ss, path);

//         // Remove leading space
//         if (!path.empty() && path[0] == ' ')
//             path = path.substr(1);

//         if (path.empty())
//             return "ERROR: Missing file path";

//         bool ok = ProcessModule::start(path);
//         return ok ? "SUCCESS: Process started" : "ERROR: Process start failed (Path incorrect)";
//     }

//     return "ERROR: Unknown process command: " + args;
// }


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

    // ---- START <path> ----
    if (action == "start") {
        std::string path;
        std::getline(ss, path);

        if (!path.empty() && path[0] == ' ')
            path = path.substr(1);

        if (path.empty())
            return "ERROR: Missing file path";

        bool ok = ProcessModule::start(path);
        return ok ? "SUCCESS: Process started" : "ERROR: Process start failed (Path incorrect)";
    }

    return "ERROR: Unknown process command: " + args;
}