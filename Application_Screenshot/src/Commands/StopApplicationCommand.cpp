#include "../../include/Commands/StopApplicationCommand.h"

std::string StopApplicationCommand::execute(const std::string& args) {
    if (args.empty()) {
        return "Error: Please specify the application name or path to stop.";
    }
    bool success = app->stopApplication(args);
    if (success) {
        return "Success: Application '" + args + "' has been stopped.";
    } else {
        return "Error: Could not stop application '" + args + "'. It may not be running or Access is Denied.";
    }
}