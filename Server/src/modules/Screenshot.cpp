#include "Screenshot.h"
#include <ShellScalingApi.h> 
#pragma comment(lib, "Shcore.lib")
#ifdef _WIN32

#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>

// Ensure that gdi32.lib is linked in case of linker errors.
bool Screenshot::take(const std::string& filename) {

    // get  virtual screen dimensions
    int vx = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int vy = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int vw = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int vh = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    if (vw <= 0 || vh <= 0) return false;

    // get device context of the screen
    HDC hScreen = GetDC(nullptr);
    if (!hScreen) return false;

    // create a memory device context compatible with the screen
    HDC hMemDC = CreateCompatibleDC(hScreen);
    if (!hMemDC) {
        ReleaseDC(nullptr, hScreen);
        return false;
    }

    // create a compatible bitmap
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, vw, vh);
    if (!hBitmap) {
        DeleteDC(hMemDC);
        ReleaseDC(nullptr, hScreen);
        return false;
    }

    // choose the new bitmap into the memory DC
    HBITMAP oldBmp = (HBITMAP)SelectObject(hMemDC, hBitmap);

    // capture the screen into the bitmap
    if (!BitBlt(hMemDC, 0, 0, vw, vh, hScreen, vx, vy, SRCCOPY | CAPTUREBLT)) {
        // clear if failed
        SelectObject(hMemDC, oldBmp);
        DeleteObject(hBitmap);
        DeleteDC(hMemDC);
        ReleaseDC(nullptr, hScreen);
        return false;
    }

    //save HBITMAP to file
    bool ok = SaveBitmapToBMPFile(hBitmap, filename);

    // clear up
    SelectObject(hMemDC, oldBmp);
    DeleteObject(hBitmap);
    DeleteDC(hMemDC);
    ReleaseDC(nullptr, hScreen);
    return ok;
}

std::string Screenshot::captureToBuffer() {
    // Force the system to regconize the correct actual resolution
    SetProcessDPIAware();

    // Take the size of virtual screen
    int vx = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int vy = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int vw = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int vh = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    HDC hScreenDC = GetDC(NULL);
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

    // Create bitmap 
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, vw, vh);
    HGDIOBJ hOldBitmap = SelectObject(hMemoryDC, hBitmap);

    // Capture with flag CAPTUREBLT to take all the menu screen
    BitBlt(hMemoryDC, 0, 0, vw, vh, hScreenDC, vx, vy, SRCCOPY | CAPTUREBLT);

    BITMAPINFOHEADER infoHeader = { 0 };
    infoHeader.biSize = sizeof(BITMAPINFOHEADER);
    infoHeader.biWidth = vw;
    infoHeader.biHeight = vh;
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 32;
    infoHeader.biCompression = BI_RGB;

    int rowStride = ((vw * 32 + 31) / 32) * 4;
    DWORD dwBmpSize = rowStride * vh;

    std::vector<char> bmpBuffer(dwBmpSize);

    // Take the real pixels
    if (GetDIBits(hScreenDC, hBitmap, 0, vh, bmpBuffer.data(), (BITMAPINFO*)&infoHeader, DIB_RGB_COLORS) == 0) {
        // Cleanup if fail...
        SelectObject(hMemoryDC, hOldBitmap);
        DeleteObject(hBitmap);
        DeleteDC(hMemoryDC);
        ReleaseDC(NULL, hScreenDC);
        return "";
    }

    BITMAPFILEHEADER fileHeader = { 0 };
    fileHeader.bfType = 0x4D42; // 'BM'
    fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    fileHeader.bfSize = fileHeader.bfOffBits + dwBmpSize;

    std::string finalBuffer;
    finalBuffer.reserve(fileHeader.bfSize);
    finalBuffer.append((char*)&fileHeader, sizeof(BITMAPFILEHEADER));
    finalBuffer.append((char*)&infoHeader, sizeof(BITMAPINFOHEADER));
    finalBuffer.append(bmpBuffer.data(), dwBmpSize);

    // Cleanup
    SelectObject(hMemoryDC, hOldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreenDC);

    return finalBuffer;
}

// Hàm phụ: Lưu HBITMAP thành file BMP
bool Screenshot::SaveBitmapToBMPFile(HBITMAP hBitmap, const std::string& filename) {
    BITMAP bmp;
    // Lấy thông tin bitmap
    if (!GetObject(hBitmap, sizeof(BITMAP), &bmp)) return false;

    int width = bmp.bmWidth;
    int height = bmp.bmHeight;
    int bpp = 32; // Lưu 32-bit (BGRA)
    int rowBytes = ((width * bpp + 31) / 32) * 4;
    int imageSize = rowBytes * height;

    // --- BMP File Header ---
    BITMAPFILEHEADER bfh;
    ZeroMemory(&bfh, sizeof(bfh));
    bfh.bfType = 0x4D42;
    bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bfh.bfSize = bfh.bfOffBits + imageSize;

    // --- BMP Info Header ---
    BITMAPINFOHEADER bih;
    ZeroMemory(&bih, sizeof(bih));
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biWidth = width;
    bih.biHeight = height;
    bih.biPlanes = 1;
    bih.biBitCount = (WORD)bpp;
    bih.biCompression = BI_RGB;
    bih.biSizeImage = imageSize;

    HDC hdc = GetDC(nullptr);
    if (!hdc) return false;

    // Cần DC để dùng GetDIBits
    HDC memDC = CreateCompatibleDC(hdc);
    if (!memDC) {
        ReleaseDC(nullptr, hdc);
        return false;
    }

    HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, hBitmap);
    std::vector<char> pixels(imageSize);

    BITMAPINFO bi;
    ZeroMemory(&bi, sizeof(bi));
    bi.bmiHeader = bih;

    // Lấy pixel raw từ HBITMAP
    if (!GetDIBits(memDC, hBitmap, 0, height, pixels.data(), &bi, DIB_RGB_COLORS)) {
        SelectObject(memDC, oldBmp);
        DeleteDC(memDC);
        ReleaseDC(nullptr, hdc);
        return false;
    }

    // --- Ghi ra file ---
    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs) {
        // Lỗi khi mở file
        SelectObject(memDC, oldBmp);
        DeleteDC(memDC);
        ReleaseDC(nullptr, hdc);
        return false;
    }

    ofs.write(reinterpret_cast<const char*>(&bfh), sizeof(bfh));
    ofs.write(reinterpret_cast<const char*>(&bih), sizeof(bih));
    ofs.write(pixels.data(), imageSize);
    ofs.close();

    // Cleanup
    SelectObject(memDC, oldBmp);
    DeleteDC(memDC);
    ReleaseDC(nullptr, hdc);
    return true;
}

#endif // _WIN32