#include "../../include/Commands/StartApplicationCommand.h"

std::string StartApplicationCommand::execute(const std::string& args) {
    if (args.empty()) return "Error: Path is empty.";
    
    auto pidOpt = app->startApplication(args);
    if (pidOpt) {
        return "Success: Started application with PID: " + std::to_string(*pidOpt);
    } else {
        return "Failed: Could not start application: " + args;
    }
}