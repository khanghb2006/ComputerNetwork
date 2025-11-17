#include "../../include/Commands/ExportApplicationCommand.h"

std::string ExportApplicationCommand::execute(const std::string& args) {
    std::string path = args.empty() ? "applications.txt" : args;
    if (app->exportApplicationToFile(path)) {
         return "Success: Exported application list to " + path;
    } else {
         return "Failed: Could not write to file " + path;
    }
}