#include "../../include/CommandFactory.h"
#include "../../include/Commands/KeyCommand.h"
#include "../../include/Commands/ProcessCommand.h"

ICommand* CommandFactory::create(const std::string& name) {
    if (name == "key") {       // Người dùng gõ 'key'
        return new KeyCommand();
    }
    if (name == "process") {   // Người dùng gõ 'process'
        return new ProcessCommand();
    }
    return nullptr;
}