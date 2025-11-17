#pragma once
#include "../ICommand.h"
#include "../../include/modules/Application.h"

class StopApplicationCommand : public ICommand {
private:
    Application* app;
public:
    StopApplicationCommand(Application* _app) : app(_app) {}
    std::string execute(const std::string& args) override;
};