#include "ProcessCommand.h"
#include "ProcessManager.h"
#include <iostream>
#include <sstream>
#include <vector>

// change vector of ProcessInfo to string for display
std::string ProcessInfoVectorToString(const std::vector<ProcessInfo>& list) {
    std::stringstream ss;
    ss << "PID\t\tNAME\n";
    ss << "--------------------------------------------------\n";
    for (const auto& info : list) {
        // Ensure correct access to struct ProcessInfo
        ss << info.pid << "\t\t" << info.name << "\n";
    }
    return ss.str();
}


std::string ProcessCommand::execute(const std::string& args) {
    std::stringstream ss(args);
    std::string action;
    ss >> action;

    std::cout << action << "\n";

    // ---- LIST ----
    if (action == "list") {

		// 1. Call module to get process data
        auto list = ProcessModule::list();

		// 2. Convert to string and return
        if (!list.empty()) {
            std::string process_data = ProcessInfoVectorToString(list);
            return process_data;
        }
        else {
            return "ERROR: Process list is empty or could not be retrieved.";
        }
    }

    // ---- KILL <PID> ----
    if (action == "stop") {
        unsigned long pid;
        ss >> pid;

        if (!ss) return "ERROR: Invalid PID";

        bool ok = ProcessModule::stop(pid);
        return ok ? "SUCCESS: Process killed" : "ERROR: Process kill failed (Access Denied or PID not found)";
    }

    return "ERROR: Unknown process command: " + args;
}