#include "modules/ocr_processor.hpp"
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <iostream>

std::string OCRProcessor::extractFullText(const cv::Mat& image) {
    tesseract::TessBaseAPI tess;
    tess.Init(nullptr, "eng", tesseract::OEM_LSTM_ONLY);
    tess.SetPageSegMode(tesseract::PSM_SINGLE_BLOCK);
    
    cv::Mat gray;
    if (image.channels() > 1) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }

    tess.SetImage(gray.data, gray.cols, gray.rows, 1, gray.step);
    char* outText = tess.GetUTF8Text();
    std::string fullText(outText ? outText : "");
    delete[] outText;

    // Debug output - show raw OCR results
    std::cout << "==================== RAW OCR OUTPUT ====================\n";
    std::cout << fullText << "\n";
    std::cout << "=======================================================\n";

    return fullText;
}

