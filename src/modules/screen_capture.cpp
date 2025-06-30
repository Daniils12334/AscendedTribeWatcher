#include "modules/screen_capture.hpp"
#include <X11/Xutil.h>
#include <iostream>

int ScreenCapture::getRightMonitorXOffset() {
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        std::cerr << "ERROR: Failed to open X display" << std::endl;
        return 0;
    }

    int screen_offset = 0;
    if (XineramaIsActive(display)) {
        int numScreens = 0;
        XineramaScreenInfo* screens = XineramaQueryScreens(display, &numScreens);
        if (screens && numScreens > 1) {
            screen_offset = screens[0].width;
            std::cout << "DEBUG: Multi-monitor setup detected. Right screen starts at X: " 
                      << screen_offset << std::endl;
        }
        if (screens) XFree(screens);
    }
    XCloseDisplay(display);
    return screen_offset;
}

cv::Mat ScreenCapture::captureGameLogRegion() {
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        std::cerr << "ERROR: XOpenDisplay() failed" << std::endl;
        return cv::Mat();
    }

    Window root = DefaultRootWindow(display);
    if (!root) {
        std::cerr << "ERROR: Failed to get root window" << std::endl;
        XCloseDisplay(display);
        return cv::Mat();
    }

    int x = getRightMonitorXOffset() + 770;
    int y = 215;
    int width = 375;
    int height = 500;

    std::cout << "DEBUG: Attempting to capture at X:" << x << " Y:" << y 
              << " W:" << width << " H:" << height << std::endl;

    XImage* img = XGetImage(display, root, x, y, width, height, AllPlanes, ZPixmap);
    if (!img) {
        std::cerr << "ERROR: XGetImage() failed" << std::endl;
        XCloseDisplay(display);
        return cv::Mat();
    }

    // Convert XImage to OpenCV Mat (BGRA format)
    cv::Mat bgraFrame(height, width, CV_8UC4, img->data);
    cv::Mat processedFrame;
    
    // Process the image (with error checking at each step)
    try {
        // Convert to BGR first
        cv::Mat bgrFrame;
        cv::cvtColor(bgraFrame, bgrFrame, cv::COLOR_BGRA2BGR);
        
        // Split channels
        std::vector<cv::Mat> channels;
        cv::split(bgrFrame, channels);
        
        // Enhanced color processing:
        // 1. Boost red channel (for red/purple text)
        cv::Mat redEnhanced;
        cv::multiply(channels[2], 2.0, redEnhanced); // Double the red channel
        
        // 2. Combine with green channel (for other colored text)
        cv::addWeighted(redEnhanced, 0.7, channels[1], 0.3, 0, processedFrame);
        
        // 3. Apply threshold to enhance contrast
        cv::threshold(processedFrame, processedFrame, 100, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
        
        #ifdef DEBUG
        cv::imwrite("debug_capture.png", processedFrame);
        #endif
    } catch (const cv::Exception& e) {
        std::cerr << "ERROR: Image processing failed: " << e.what() << std::endl;
        processedFrame = bgraFrame; // Fallback to original if processing fails
    }

    XDestroyImage(img);
    XCloseDisplay(display);

    if (processedFrame.empty()) {
        std::cerr << "ERROR: Final processed frame is empty" << std::endl;
        return cv::Mat();
    }

    std::cout << "DEBUG: Capture and processing successful. Image size: " 
              << processedFrame.cols << "x" << processedFrame.rows << std::endl;
    return processedFrame;
}