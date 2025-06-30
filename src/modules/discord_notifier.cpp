#include "modules/discord_notifier.hpp"
#include <curl/curl.h>
#include <iostream>

bool DiscordNotifier::sendLog(const std::string& message) {
    CURL* curl = curl_easy_init();
    if (curl) {
        const std::string webhookUrl = "https://discord.com/api/webhooks/1334124265104085044/i4F99g9T1-5rydCr7qbgJ5WHLCPkdpR-098MCv4eFU9BJzwCTvrp7IeIe96_ICTepj5K";
        std::string jsonPayload = R"({"content": ")" + message + R"("})";

        curl_easy_setopt(curl, CURLOPT_URL, webhookUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonPayload.c_str());
        
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);

        return res == CURLE_OK;
    }
    return false;
}