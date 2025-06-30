#pragma once
#include <string>

class DiscordNotifier {
public:
    static bool sendLog(const std::string& message);
};