#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <regex>
#include <iomanip>
#include <sstream>

class LogProcessor {
public:
    static std::string extractMostRecentLog(const std::string& fullText);
    static std::string extractNormalizedTimestamp(const std::string& log);
    static std::string formatLog(const std::string& rawLog);
    static bool isValidLog(const std::string& text);
    static std::string highlightImportantEvents(const std::string& formattedLog);
    
private:
    static const std::unordered_map<std::string, std::string> wordSeparations;
    static const std::vector<std::string> importantKeywords;
};