#include "../../include/Commands/ApplicationCommand.h"
#include <sstream>
#include <iostream>
#include <vector>

// Hàm dọn dẹp chuỗi JSON
std::string escapeJson(const std::string& s) {
    std::string res;
    for (char c : s) {
        if (c == '\\') res += "\\\\";
        else if (c == '"') res += "\\\"";
        else if (c == '\n' || c == '\r') res += " ";
        else if (c >= 32 && c <= 126) res += c; // Chỉ lấy ký tự in được
        else res += '?';
    }
    return res;
}

std::string ApplicationCommand::execute(const std::string& args) {
    std::stringstream ss(args);
    std::string action;
    ss >> action; 

    // --- LIST COMMAND ---
    if (action == "list") {
        // Lấy danh sách nhưng KHÔNG in ra Console của Server để tránh rác
        std::vector<ApplicationInfo> list = ApplicationManager::listApplication(); 
        
        std::stringstream json;
        json << "{\"type\":\"PROCESS_LIST\",\"data\":[";
        
        bool first = true;
        for (const auto& app : list) {
            if (app.name().empty()) continue;

            if (!first) json << ",";
            json << "{";
            json << "\"pid\":" << app.pid() << ",";
            json << "\"name\":\"" << escapeJson(app.name()) << "\",";
            json << "\"path\":\"" << escapeJson(app.path()) << "\""; 
            json << "}";
            first = false;
        }
        json << "]}";
        
        // Chỉ trả về chuỗi JSON sạch
        return "JSON_LIST:" + json.str(); 
    }
    
    // --- START COMMAND ---
    if (action == "start") {
        std::string path;
        if (ss.peek() == ' ') ss.ignore(1);
        std::getline(ss, path); 
        auto pidOpt = ApplicationManager::startApplication(path);
        return pidOpt ? "CMD_MSG:Started PID " + std::to_string(*pidOpt) : "CMD_MSG:Start Failed";
    }

    // --- STOP COMMAND ---
    if (action == "stop") {
        std::string target;
        if (ss.peek() == ' ') ss.ignore(1);
        std::getline(ss, target); 
        bool ok = ApplicationManager::stopApplication(target); 
        return ok ? "CMD_MSG:Stopped " + target : "CMD_MSG:Stop Failed";
    }
    
    return "CMD_MSG:Unknown command";
}