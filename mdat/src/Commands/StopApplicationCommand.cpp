#include "../../include/Commands/StopApplicationCommand.h"

std::string StopApplicationCommand::execute(const std::string& args) {
    try {
        int pid = std::stoi(args);
        if (app->stopApplication(pid)) {
            return "Success: Stopped application with PID " + args;
        } else {
            return "Failed: Could not stop application " + args;
        }
    } catch (...) {
        return "Error: Invalid PID format.";
    }
}