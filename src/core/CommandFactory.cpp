// CommandFactory.cpp
// Giả sử file này nằm trong src/core/

#include "../../include/CommandFactory.h"

// Bỏ các include Command riêng lẻ (List, Stop, Start)
// #include "../../include/Commands/ListApplicationCommand.h"
// ...

// Thêm include cho Command mới đã hợp nhất
#include "../../include/Commands/ApplicationCommand.h" 
#include "../../include/Commands/ScreenshotCommand.h" // Giữ nguyên cho Screenshot

ICommand* CommandFactory::create(const std::string& cmdName) { 
    // 1. Lệnh Ứng dụng/Process (Gộp chung)
    if (cmdName == "application" || cmdName == "process") {
        return new ApplicationCommand();
    }
    // 2. Lệnh Screenshot
    else if (cmdName == "screenshot") {
        return new ScreenshotCommand();
    }
    // 3. Lệnh Keylogger (Chưa triển khai)
    else if (cmdName == "keylogger") {
        // return new KeyloggerCommand();
    }
    // 4. Lệnh Webcam (Chưa triển khai)
    else if (cmdName == "webcam") {
        // return new WebcamCommand();
    }
    // 5. Lệnh System (Shutdown/Restart) (Gộp chung)
    else if (cmdName == "system") {
        // return new SystemCommand(); 
    }
    return nullptr;
}