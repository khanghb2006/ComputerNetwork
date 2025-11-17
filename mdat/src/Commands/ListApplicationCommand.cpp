#include "../../include/Commands/ListApplicationCommand.h"
#include <sstream>

std::string ListApplicationCommand::execute(const std::string& args) {
    auto list = app->listApplication();
    std::stringstream ss;
    ss << "--- Running Applications ---\n";
    for (const auto& p : list) {
        ss << "PID: " << p.pid() << " - " << p.name() << "\n";
        ss << "  Path: " << p.path() << "\n";
    }
    ss << "--------------------------------";
    return ss.str();
}