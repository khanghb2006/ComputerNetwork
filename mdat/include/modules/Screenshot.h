#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>

class Screenshot {
public:
    Screenshot() = default;

    // Save full virtual screen to BMP file. Returns true on success.
    bool take(const std::string& filename);

private:
    static bool SaveBitmapToBMPFile(HBITMAP hBitmap, const std::string& filename);
};
#endif // _WIN32
