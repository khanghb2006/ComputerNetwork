#include "ApplicationCommand.h"
#include <sstream>
#include <iostream>
#include <vector>

// clear JSON string 
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
    std::string action , path;
    std::cout << args << "\n";
    ss >> action;

    // LIST COMMAND
    if (action == "list") {
		// get the list of applications
        std::vector<ApplicationInfo> list = ApplicationManager::listApplication();

        std::stringstream json;
        json << "{\"type\":\"APPLICATION_LIST\",\"data\":[";

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

		return json.str(); // JSON string
    }

	// START COMMAND
    if (action == "start") {
        std::string path;
        ss >> path;
        std::cout << path << "\n";
        auto pidOpt = ApplicationManager::startApplication(path);
        return pidOpt ? "CMD_MSG:Started PID " + std::to_string(*pidOpt) : "CMD_MSG:Start Failed";
    }

	// STOP COMMAND
    if (action == "stop") {
        std::string target;
        ss >> target;
        bool ok = ApplicationManager::stopApplication(target);
        return ok ? "CMD_MSG:Stopped " + target : "CMD_MSG:Stop Failed";
    }

	return "CMD_MSG:Unknown command"; // default response
}