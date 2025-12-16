#pragma once
#include <string>

class Record {
	private:
		int cameraIdx; // Index of the camera
		int duration; // Duration of the recording in seconds
		int fps; // Frames per second
		std::string path; // File path to save the recording

	public:
		// Constructor
		Record(int camIdx = 0, int dur = 10, int fps = 30, const std::string& p = "recording.avi");

	public:
		bool startRecord();

	public:
		~Record() {}
};