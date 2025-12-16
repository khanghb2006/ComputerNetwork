#pragma once
#include <string>

class KeyController {
public:
    static KeyController& getInstance();

    // Bắt đầu bắt phím (auto spawns hook/thread/loop)
    bool startCapture();

    // Dừng bắt phím
    bool stopCapture();

    // Lấy buffer và clear
    std::string getBufferAndClear();
private:
    KeyController();
    ~KeyController();
    KeyController(const KeyController&) = delete;
    KeyController& operator=(const KeyController&) = delete;

    struct Impl;
    Impl* pImpl;
};


class KeyControllerModule {
public:
    // start capturing keys (returns true if capture started)
    static bool start();

    // stop capturing keys (returns true if stopped successfully)
    static bool stop();

    // get buffer contents and clear internal buffer
    static std::string getBuffer();
};