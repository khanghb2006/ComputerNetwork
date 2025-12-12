#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>

class Screenshot {
public:
    Screenshot() = default;

    // Lưu tất cả ảnh chụp màn hình thành BMP file
    bool take(const std::string& filename);

    // Cap màn hình và return buffer của BMPs
    std::string captureToBuffer();

private:
    static bool SaveBitmapToBMPFile(HBITMAP hBitmap, const std::string& filename);
};
#endif // _WIN32
