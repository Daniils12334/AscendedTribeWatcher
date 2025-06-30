#include "modules/log_processor.hpp"
#include "modules/discord_notifier.hpp"
#include "modules/ocr_processor.hpp"
#include "modules/screen_capture.hpp"
#include <iostream>
#include <deque>
#include <unordered_set>
#include <chrono>
#include <thread>

// Log tracking system
std::unordered_set<std::string> recentLogs;
std::unordered_set<std::string> sentTimestamps;
std::deque<std::string> logBuffer;
const int MAX_LOG_BUFFER_SIZE = 100;
bool isDisconnectedSent = false;


int main() {
    std::cout << "Initializing GAMMA Logger (Final Solution)..." << std::endl;
    
    try {
        DiscordNotifier::sendLog("🔌 GAMMA Logger starting in FINAL mode...");
        
        while (true) {
            try {
                // 1. Capture entire log region
                cv::Mat frame = ScreenCapture::captureGameLogRegion();
                if (frame.empty()) {
                    if (!isDisconnectedSent) {
                        DiscordNotifier::sendLog("⚠️ **DISCONNECTED** - Screen capture failed");
                        isDisconnectedSent = true;
                    }
                    std::this_thread::sleep_for(std::chrono::seconds(2));
                    continue;
                }

                // 2. Extract full OCR text
                std::string fullText = OCRProcessor::extractFullText(frame);
                std::cout << "FULL OCR TEXT:\n" << fullText << "\n";
                
                // 3. Extract ONLY the most recent log
                std::string rawLog = LogProcessor::extractMostRecentLog(fullText);
                std::cout << "MOST RECENT LOG: " << rawLog << "\n";
                
                // 4. Format log
                std::string formattedLog = LogProcessor::formatLog(rawLog);
                
                // 5. Extract timestamp for duplicate checking
                std::string timestamp = LogProcessor::extractTimestamp(formattedLog);
                if (timestamp.empty()) {
                    std::cout << "NO TIMESTAMP - Skipping\n";
                    continue;
                }

                // 6. Check if log is new
                if (sentTimestamps.find(timestamp) == sentTimestamps.end()) {
                    std::cout << "NEW LOG DETECTED: " << formattedLog << "\n";
                    
                    // Highlight important events
                    std::string enhancedLog = LogProcessor::highlightImportantEvents(formattedLog);
                    
                    if (DiscordNotifier::sendLog(enhancedLog)) {
                        std::cout << "Successfully sent to Discord\n";
                        sentTimestamps.insert(timestamp);
                        
                        // Reset disconnect flag
                        isDisconnectedSent = false;
                    }
                } else {
                    std::cout << "DUPLICATE LOG - Already processed\n";
                }

                // 7. Rate limiting - crucial to prevent spamming
                std::this_thread::sleep_for(std::chrono::seconds(1));
            } catch (const std::exception& e) {
                std::cerr << "Loop error: " << e.what() << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(2));
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        DiscordNotifier::sendLog("💀 **CRASH** - " + std::string(e.what()));
        return 1;
    }
    return 0;
}