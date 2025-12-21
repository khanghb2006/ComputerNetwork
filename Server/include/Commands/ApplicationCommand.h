#pragma once
#include "ICommand.h"
#include "ApplicationManager.h" 

class ApplicationCommand : public ICommand {
public:
    ApplicationCommand() = default;
    std::string execute(const std::string& args) override;
};