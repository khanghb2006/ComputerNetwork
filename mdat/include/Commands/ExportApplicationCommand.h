#pragma once
#include "../ICommand.h"
#include "../../include/modules/Application.h"

class ExportApplicationCommand : public ICommand {
private:
    Application* app;
public:
    ExportApplicationCommand(Application* _app) : app(_app) {}
    std::string execute(const std::string& args) override;
};