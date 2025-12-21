#include "ScreenshotCommand.h"
#include "Screenshot.h"
#include <iostream>
#include <vector>
#include <string>

static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

std::string base64_encode(const unsigned char* bytes_to_encode, unsigned int in_len) {
    std::string ret;
    int i = 0, j = 0;
    unsigned char char_array_3[3], char_array_4[4];
    while (in_len--) {
		// read each byte from input and put into char_array_3
        char_array_3[i++] = *(bytes_to_encode++);

		// when we have 3 bytes, encode them to 4 base64 characters
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            for (i = 0; (i < 4); i++) ret += base64_chars[char_array_4[i]];
			i = 0; // reset for next 3 bytes
        }
    }
	// handle padding
    if (i) {
        for (j = i; j < 3; j++) char_array_3[j] = '\0';
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;
        for (j = 0; (j < i + 1); j++) ret += base64_chars[char_array_4[j]];
        while ((i++ < 3)) ret += '='; // add padding
    }
    return ret;
}

std::string ScreenshotCommand::execute(const std::string& args) {
#ifdef _WIN32
    Screenshot screenshot;

	// 1. Use captureToBuffer (Get image into RAM, DO NOT save file)
    std::string bmpBuffer = screenshot.captureToBuffer();

    if (bmpBuffer.empty()) {
        return "CMD_MSG:Loi - Khong chup duoc man hinh.";
    }

    // 2. encoding Base64
    std::string encoded = base64_encode(
        reinterpret_cast<const unsigned char*>(bmpBuffer.data()),
        bmpBuffer.size()
    );

	// add EOF marker
    return encoded + "__EOF__";

#else
    return "CMD_MSG:Not supported.";
#endif
}