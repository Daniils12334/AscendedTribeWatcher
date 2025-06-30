#include "modules/log_processor.hpp"
#include <iostream>
#include <algorithm>
#include <cctype>

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
    {"D8", "08"}  // Fix OCR error for timestamp
};

// Keywords that trigger important event highlighting
const std::vector<std::string> LogProcessor::importantKeywords = {
    "destroyed", "killed", "demolished", "starved", 
    "expired", "froze", "died", "slain", "decay"
};

std::string LogProcessor::extractMostRecentLog(const std::string& fullText) {
    // Find the position of the first timestamp
    size_t timestampPos = fullText.find("Day ");
    if (timestampPos == std::string::npos) {
        timestampPos = fullText.find("day ");
        if (timestampPos == std::string::npos) {
            return fullText;
        }
    }

    // Find the position of the next timestamp
    size_t nextTimestampPos = fullText.find("Day ", timestampPos + 4);
    if (nextTimestampPos == std::string::npos) {
        nextTimestampPos = fullText.find("day ", timestampPos + 4);
    }

    // If we found a next timestamp, extract up to that point
    if (nextTimestampPos != std::string::npos) {
        return fullText.substr(timestampPos, nextTimestampPos - timestampPos);
    }
    
    // Otherwise return from the first timestamp to the end
    return fullText.substr(timestampPos);
}

std::string LogProcessor::extractTimestamp(const std::string& log) {
    // Simplified timestamp extraction - find first colon
    size_t colonPos = log.find(':');
    if (colonPos == std::string::npos || colonPos < 10 || colonPos > 30) {
        return "";
    }

    // Extract text from start to 2 characters after first colon
    size_t start = 0;
    size_t end = log.find(' ', colonPos + 3);
    if (end == std::string::npos) {
        end = log.length();
    }

    std::string timestamp = log.substr(start, end - start);
    
    // Fix common OCR errors in timestamp
    size_t pos = 0;
    while ((pos = timestamp.find("D8", pos)) != std::string::npos) {
        timestamp.replace(pos, 2, "08");
        pos += 2;
    }
    
    return timestamp;
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