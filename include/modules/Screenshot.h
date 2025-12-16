#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>

class Screenshot {
public:
    Screenshot() = default;

	// save all screenshots to BMP files
    bool take(const std::string& filename);

	// return BMP data as string buffer
    std::string captureToBuffer();

private:
    static bool SaveBitmapToBMPFile(HBITMAP hBitmap, const std::string& filename);
};
#endif 