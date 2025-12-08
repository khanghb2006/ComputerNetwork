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
    if (cmdName == "application") { 
        return new ApplicationCommand();
    }
    else if (cmdName == "screenshot") {
        return new ScreenshotCommand();
    }

    return nullptr;
}