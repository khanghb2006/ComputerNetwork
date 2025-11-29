#include "../../include/Commands/ListApplicationCommand.h"
#include <sstream>

std::string ListApplicationCommand::execute(const std::string& args) {
    std::string defaultFile = "runningApp.txt";
    bool success = app->exportApplicationToFile(defaultFile);
    if (success) {
        return "Success: List of running applications has been saved to '" + defaultFile + "'.";
    } else {
        return "Failed: Failed to save application list to '" + defaultFile + "'.";
    }
}