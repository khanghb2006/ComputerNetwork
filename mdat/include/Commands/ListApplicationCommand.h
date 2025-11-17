#pragma once
#include "../ICommand.h"
#include "../../include/modules/Application.h" // Trỏ về Application.h

class ListApplicationCommand : public ICommand {
private:
    Application* app;
public:
    ListApplicationCommand(Application* _app) : app(_app) {}
    std::string execute(const std::string& args) override;
};