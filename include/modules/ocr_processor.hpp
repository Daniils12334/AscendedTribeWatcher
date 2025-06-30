#pragma once
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

class OCRProcessor {
public:
    static std::string extractFullText(const cv::Mat& image);
    
private:
    // No need for separate extractLogsFromImage anymore
};