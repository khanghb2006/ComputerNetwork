#pragma once
#include <opencv2/opencv.hpp>
#include <string>

class Webcam {
	private:
		cv::VideoCapture cap; // OpenCV VideoCapture object
		bool isOpened; // Flag to check if the webcam is opened
		int cameraIdx; // Index of the camera
	
	public:
		Webcam(int idx = 0);
		~Webcam();

	public:
		bool open();
		void close();
		bool isOpen() const;

	public:
		// Start recording from the webcam
		bool recordVideo(const std::string& path, int duration = 10, int fps = 30);

		void showPreview();
};