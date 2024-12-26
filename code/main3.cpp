#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::vector<std::string> extractMessages(const std::string jsonString) {
    std::vector<std::string> messages;
    try {
        json jsonObject = json::parse(jsonString);

        if (jsonObject.contains("choices")) {
            for (const auto& choice : jsonObject["choices"]) {
                if (choice.contains("message") && choice["message"].contains("content")) {
                    std::string message = choice["message"]["content"];
                    messages.push_back(message);
                }
            }
        }
    } catch (const json::parse_error& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return messages;
}


size_t ai_return(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    std::string* response = static_cast<std::string*>(userp);
    const size_t MAX_RESPONSE_SIZE = 10 * 1024 * 1024; // 10 MB
    if (response->size() + totalSize > MAX_RESPONSE_SIZE) {
        std::cerr << "Response size exceeds maximum limit of " << MAX_RESPONSE_SIZE << " bytes." << std::endl;
        return 0; // Stop processing
    }

    try {
        response->append(static_cast<char*>(contents), totalSize);
    } catch (const std::bad_alloc&) {
        std::cerr << "Memory allocation failed while appending response." << std::endl;
        return 0; // Stop processing
    }

    return totalSize;
}

std::string apiHandler(const std::string& input) {
    std::string api_url = "https://api.mistral.ai/v1/agents/completions";
    std::string api_key = "pnEsk3HSre1fvAoyX4A8SLS0kIcaYQal";
    std::string response;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL* handle = curl_easy_init();

    if (handle) {
        nlohmann::json payload;
        payload["messages"] = {{{"role", "user"}, {"content", input}}};
        payload["agent_id"] = "ag:9f2abef3:20241121:search-recommender:cf213f1b";
        std::string payloadString = payload.dump();
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "Accept: application/json");
        headers = curl_slist_append(headers, ("Authorization: Bearer " + api_key).c_str());

        curl_easy_setopt(handle, CURLOPT_URL, api_url.c_str());
        curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(handle, CURLOPT_POSTFIELDS, payloadString.c_str());
        curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, ai_return);
        curl_easy_setopt(handle, CURLOPT_WRITEDATA, &response);
        CURLcode res = curl_easy_perform(handle);
        if (res != CURLE_OK) {
            std::cerr << "cURL error: " << curl_easy_strerror(res) << std::endl;
            exit(EXIT_FAILURE);
        } else {
            std::cout << "Response from Mistral.ai:  " << response << std::endl;
            return response;
        }
        curl_easy_cleanup(handle);
        curl_slist_free_all(headers);
    }

    curl_global_cleanup();
}
