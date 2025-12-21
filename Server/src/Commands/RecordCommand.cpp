#include "RecordCommand.h"
#include "../modules/Record.h"
#include <iostream>
#include <string>
#include <filesystem>
#include <cstdlib>

bool convertAviToMp4(const std::string& aviPath, const std::string& mp4Path) {
    if (!std::filesystem::exists(aviPath)) {
        std::cout << "AVI NOT FOUND\n";
        return false;
    }

    std::filesystem::create_directories(
        std::filesystem::path(mp4Path).parent_path()
    );

    std::string cmd =
        "\"C:/ffmpeg/bin/ffmpeg.exe\""
        " -y -i \"" + aviPath + "\""
        " -c:v libx264 -preset fast -crf 23 "
        "\"" + mp4Path + "\"";

    std::string fullCmd = "cmd /C \"" + cmd + "\"";

    std::cout << "[FFMPEG CMD] " << fullCmd << std::endl;

    system(fullCmd.c_str());

    return std::filesystem::exists(mp4Path);
}

std::string RecordCommand::execute(const std::string&) {
    int duration = 10;
    int fps = 30;

    std::string avi = "D:/Project_ComputerNetwork/Server/Video/record.avi";
    std::string mp4 = "D:/Project_ComputerNetwork/Server/Video/record.mp4";

    Record rec(0, duration, fps, avi);

    if (!rec.startRecord())
        return "RECORD_FAILED";

	// convert to mp4 because browser support mp4 better than avi
    if (!convertAviToMp4(avi, mp4))
        return "CONVERT_FAILED";

    return mp4;
}