#include "../../include/Commands/ScreenshotCommand.h"
#include "../../include/modules/Screenshot.h" // Cần include logic screenshot gốc

std::string ScreenshotCommand::execute(const std::string& args) {
#ifdef _WIN32
    Screenshot screenshot;
    std::string path = args.empty() ? "screenshot.bmp" : args;
    
    if (screenshot.take(path)) {
        return "Success: Screenshot saved to " + path;
    } else {
        return "Failed: Could not save screenshot.";
    }
#else
    return "Error: Feature not supported on this platform.";
#endif
}