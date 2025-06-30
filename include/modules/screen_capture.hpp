#pragma once
#include <opencv2/opencv.hpp>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xinerama.h>

class ScreenCapture {
public:
    static cv::Mat captureGameLogRegion();
    static int getRightMonitorXOffset();
};