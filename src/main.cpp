#include "modules/log_processor.hpp"
#include "modules/discord_notifier.hpp"
#include "modules/ocr_processor.hpp"
#include "modules/screen_capture.hpp"
#include <iostream>
#include <unordered_set>
#include <deque>
#include <chrono>
#include <thread>

// Log tracking system
std::unordered_set<std::string> sentTimestamps;
std::deque<std::string> logBuffer;
const int MAX_LOG_BUFFER_SIZE = 100;  // Store 100 most recent timestamps
bool isDisconnectedSent = false;

int main() {
    std::cout << "Initializing GAMMA Logger (Strict Date-Based Deduplication)..." << std::endl;
    
    try {
        DiscordNotifier::sendLog("🔌 GAMMA Logger starting with strict date-based deduplication...");
        
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
                
                // 3. Extract only the most recent log
                std::string rawLog = LogProcessor::extractMostRecentLog(fullText);
                std::cout << "MOST RECENT LOG: " << rawLog << "\n";
                
                // 4. Format log
                std::string formattedLog = LogProcessor::formatLog(rawLog);
                
                // Skip if log is invalid
                if (!LogProcessor::isValidLog(formattedLog)) {
                    std::cout << "INVALID LOG - Skipping\n";
                    continue;
                }

                // 5. Extract normalized timestamp
                std::string timestamp = LogProcessor::extractNormalizedTimestamp(formattedLog);
                std::cout << "NORMALIZED TIMESTAMP: " << (timestamp.empty() ? "NONE" : timestamp) << "\n";
                
                // 6. Process log based on timestamp
                if (!timestamp.empty()) {
                    // Check if we've seen this timestamp before
                    if (sentTimestamps.find(timestamp) == sentTimestamps.end()) {
                        // New log detected
                        std::cout << "NEW LOG DETECTED: " << formattedLog << "\n";
                        
                        // Highlight important events
                        std::string enhancedLog = LogProcessor::highlightImportantEvents(formattedLog);
                        
                        if (DiscordNotifier::sendLog(enhancedLog)) {
                            std::cout << "Successfully sent to Discord\n";
                            
                            // Record timestamp
                            sentTimestamps.insert(timestamp);
                            logBuffer.push_back(timestamp);
                            
                            // Maintain buffer size
                            if (logBuffer.size() > MAX_LOG_BUFFER_SIZE) {
                                std::string oldest = logBuffer.front();
                                logBuffer.pop_front();
                                sentTimestamps.erase(oldest);
                            }
                            
                            // Reset disconnect flag
                            isDisconnectedSent = false;
                        }
                    } else {
                        std::cout << "DUPLICATE LOG - Already processed\n";
                    }
                } else {
                    std::cout << "INVALID TIMESTAMP - Skipping\n";
                }

                // 7. Rate limiting
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
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