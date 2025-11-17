#include "../../include/CommandFactory.h"

#include "../../include/Commands/ListApplicationCommand.h"
#include "../../include/Commands/StopApplicationCommand.h"
#include "../../include/Commands/StartApplicationCommand.h"
#include "../../include/Commands/ExportApplicationCommand.h"
#include "../../include/Commands/ScreenshotCommand.h"

ICommand* CommandFactory::create(const std::string& cmdName, Application* app) {
    if (cmdName == "list") {
        return new ListApplicationCommand(app);
    }
    else if (cmdName == "stop") {
        return new StopApplicationCommand(app);
    }
    else if (cmdName == "start") {
        return new StartApplicationCommand(app);
    }
    else if (cmdName == "export") {
        return new ExportApplicationCommand(app);
    }
    else if (cmdName == "screenshot") {
        return new ScreenshotCommand();
    }
    return nullptr;
}