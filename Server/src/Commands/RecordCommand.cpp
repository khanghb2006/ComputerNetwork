#include "RecordCommand.h"
#include "../modules/Record.h"

std::string RecordCommand::execute(const std::string& args) {
	int duration = 10; // default 10 seconds
	int fps = 30; // default 30 fps
	std::string path = "D:/Project_ComputerNetwork/Server/Video/record.avi"; // default path
	
	Record rec(0, duration, fps, path);
	bool ok = rec.startRecord();
	return ok ? "Record Success" : "Record Failed";
}