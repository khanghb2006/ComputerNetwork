#include <iostream>
#include "Webcam.h"
#include "Record.h"

Record::Record(int camIdx , int dur , int fpsValue, const std::string& p)
	: cameraIdx(camIdx), duration(dur), fps(fpsValue), path(p) {}

bool Record::startRecord() {
	Webcam cam(cameraIdx);

	std::cout << "Starting recording" << "\n";
	std::cout << "Camera Index: " << cameraIdx << ", Duration: " 
		<< duration << "s, FPS: " << fps << ", Path: " << path << "\n";
	
	bool ok = cam.recordVideo(path, duration, fps);

	if (!ok) {
		std::cout << "Recording failed!" << "\n";
		return false;
	}
	std::cout << "Recording completed successfully!" << "\n";
	return true;
}
