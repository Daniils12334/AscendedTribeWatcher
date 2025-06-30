#include "modules/log_processor.hpp"
#include <iostream>
#include <algorithm>
#include <cctype>
#include <regex>

// Word separations for common OCR errors
const std::unordered_map<std::string, std::string> LogProcessor::wordSeparations = {
    {"foundationwas", "foundation was"},
    {"destroyedtheir", "destroyed their"},
    {"tribeof", "tribe of"},
    {"lvl", "lvl "},
    {"pin coded", "pin-coded"},
    {"wasauto", "was auto"},
    {"wasdestroyed", "was destroyed"},
    {"yourmetal", "your metal"},
    {"babygiganotosaurus", "baby giganotosaurus"},
    {"tribemember", "tribe member"},
    {"autodecay", "auto-decay"},
    {"starvedto", "starved to"},
    {"waskilled", "was killed"},
    {"tokill", "to kill"},
    {"auto decay", "auto-decay"},
    {"metal foundation", "Metal Foundation"},
    {"D4", "04"},  // Fix OCR error for hour
    {"D8", "08"}   // Fix OCR error for hour
};

// Keywords that trigger important event highlighting
const std::vector<std::string> LogProcessor::importantKeywords = {
    "destroyed", "killed", "demolished", "starved", 
    "expired", "froze", "died", "slain", "decay"
};

std::string LogProcessor::extractMostRecentLog(const std::string& fullText) {
    // Find the first timestamp in the text
    size_t startPos = fullText.find("Day ");
    if (startPos == std::string::npos) {
        startPos = fullText.find("day ");
        if (startPos == std::string::npos) {
            return fullText;  // No timestamp found
        }
    }

    // Find the next timestamp after the first one
    size_t nextPos = fullText.find("Day ", startPos + 4);
    if (nextPos == std::string::npos) {
        nextPos = fullText.find("day ", startPos + 4);
    }

    // Extract the most recent log
    if (nextPos != std::string::npos) {
        return fullText.substr(startPos, nextPos - startPos);
    }
    return fullText.substr(startPos);
}

std::string LogProcessor::extractNormalizedTimestamp(const std::string& log) {
    // Regex pattern to match various timestamp formats
    std::regex timestampPattern(R"((?:day|Day)[\s,.:]*(\d+)[\s,.:]*(\d{1,2})[\s,.:](\d{1,2})[\s,.:](\d{1,2}))");
    std::smatch matches;
    
    if (std::regex_search(log, matches, timestampPattern)) {
        try {
            int day = std::stoi(matches[1].str());
            int hour = std::stoi(matches[2].str());
            int minute = std::stoi(matches[3].str());
            int second = std::stoi(matches[4].str());
            
            // Validate time components
            if (hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 || second > 59) {
                return "";
            }
            
            // Format as standardized timestamp: DDDD:HH:MM:SS
            std::ostringstream formatted;
            formatted << day << ":"
                      << std::setw(2) << std::setfill('0') << hour << ":"
                      << std::setw(2) << std::setfill('0') << minute << ":"
                      << std::setw(2) << std::setfill('0') << second;
            return formatted.str();
        } catch (...) {
            return "";
        }
    }
    return "";
}

std::string LogProcessor::formatLog(const std::string& rawLog) {
    std::string formatted = rawLog;
    
    // Replace newlines with spaces
    std::replace(formatted.begin(), formatted.end(), '\n', ' ');
    
    // Apply word separations
    for (const auto& [bad, good] : wordSeparations) {
        size_t pos = 0;
        while ((pos = formatted.find(bad, pos)) != std::string::npos) {
            formatted.replace(pos, bad.length(), good);
            pos += good.length();
        }
    }

    // Standardize "day" to "Day"
    size_t dayPos = formatted.find("day ");
    if (dayPos != std::string::npos) {
        formatted.replace(dayPos, 3, "Day");
    }

    // Trim trailing spaces
    size_t endpos = formatted.find_last_not_of(" ");
    if (std::string::npos != endpos) {
        formatted = formatted.substr(0, endpos + 1);
    }

    return formatted;
}

bool LogProcessor::isValidLog(const std::string& text) {
    // Must contain a colon (timestamp indicator)
    return text.find(':') != std::string::npos;
}

std::string LogProcessor::highlightImportantEvents(const std::string& formattedLog) {
    for (const auto& keyword : importantKeywords) {
        if (formattedLog.find(keyword) != std::string::npos) {
            return "🚨 **" + formattedLog + "** 🚨";
        }
    }
    return formattedLog;
}