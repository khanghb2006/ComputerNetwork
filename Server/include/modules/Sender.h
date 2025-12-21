#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

#include "WSDataType.h"

bool sendAll(SOCKET s, const char* data, int len);
void sendWsFrame(SOCKET client , uint8_t opcode , bool fin , const char* data, size_t len);
void sendWsTextAuto(SOCKET client, const std::string& msg);
void sendWsAuto(SOCKET client, WSDataType type, const std::string& payload);