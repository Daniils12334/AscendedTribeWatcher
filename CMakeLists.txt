cmake_minimum_required(VERSION 3.10)
project(AscendedTribeWatcher)

set(CMAKE_CXX_STANDARD 17)

# Find dependencies
find_package(OpenCV REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(TESSERACT REQUIRED tesseract lept)
find_package(CURL REQUIRED)
find_package(X11 REQUIRED)
find_library(XINERAMA_LIB Xinerama)

# Include directories
include_directories(
    ${OpenCV_INCLUDE_DIRS}
    ${TESSERACT_INCLUDE_DIRS}
    ${X11_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/modules
)

# Add executable
add_executable(gamma_logger_advanced
    src/main.cpp
    src/modules/discord_notifier.cpp
    src/modules/log_processor.cpp
    src/modules/ocr_processor.cpp
    src/modules/screen_capture.cpp
)

# Link libraries
target_link_libraries(gamma_logger_advanced
    ${OpenCV_LIBS}
    ${TESSERACT_LIBRARIES}
    ${CURL_LIBRARIES}
    ${X11_LIBRARIES}
    ${XINERAMA_LIB}
)