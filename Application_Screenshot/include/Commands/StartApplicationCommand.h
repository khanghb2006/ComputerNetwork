#pragma once
#include "../ICommand.h"
#include "../../include/modules/Application.h"

class StartApplicationCommand : public ICommand {
private:
    Application* app;
public:
    StartApplicationCommand(Application* _app) : app(_app) {}
    std::string execute(const std::string& args) override;
};