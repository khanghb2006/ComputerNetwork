#include <iostream>
#include "Webcam.h"
#include <chrono>

Webcam::Webcam(int idx)
    : cameraIdx(idx), isOpened(false) {
}

Webcam::~Webcam() {
    close();
}

bool Webcam::open() {
    if (isOpened)
        return true;

    if (!cap.open(cameraIdx)) {
        std::cerr << "[Webcam] Cannot open camera index " << cameraIdx << "\n";
        return false;
    }

    isOpened = true;
    return true;
}

void Webcam::close() {
    if (isOpen()) {
        cap.release();
        isOpened = false;
    }
}

bool Webcam::isOpen() const {
    return isOpened;
}

bool Webcam::recordVideo(const std::string& path, int duration, int fps)
{
    if (!open()) return false;

    int w = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    int h = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));

    cv::VideoWriter writer(
        path, // VD: D:/Project_ComputerNetwork/Server/Video/record.avi
        cv::VideoWriter::fourcc('M', 'J', 'P', 'G'),
        fps,
        cv::Size(w, h)
    );

    if (!writer.isOpened()) {
        std::cerr << "[Webcam] Cannot create VideoWriter\n";
        return false;
    }

    auto start = std::chrono::steady_clock::now();
    cv::Mat frame;

    std::cout << "[Webcam] Recording... Press ESC to stop.\n";

    while (true) {
        cap >> frame;
        if (frame.empty()) break;

		// write frame to video file
        writer.write(frame);
        cv::imshow("Recording Preview", frame);

		// Exit on ESC key
        if (cv::waitKey(1) == 27) break;

		// Check stop duration
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - start).count() >= duration)
            break;
    }

    cv::destroyWindow("Recording Preview");
    writer.release();
    cap.release();
    return true;
}

void Webcam::showPreview() {
    if (!open()) return;

    cv::Mat frame;
    std::cout << "[Webcam] Preview running... Press ESC to exit.\n";

    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        cv::imshow("Preview", frame);
        if (cv::waitKey(1) == 27) break;
    }

    cv::destroyWindow("Preview");
}

