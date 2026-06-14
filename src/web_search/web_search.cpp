#include "web_search.h"
#include <iostream>
#include <sstream>
#include <curl/curl.h>
#include <unistd.h>

namespace khushi {

// Static initializer for curl
static bool curl_initialized = false;
static void initialize_curl() {
    if (!curl_initialized) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl_initialized = true;
    }
}

// Callback for curl to write response data
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

WebSearch::WebSearch() {
    initialize_curl();
}

WebSearch::~WebSearch() {
}

std::string WebSearch::make_search_request(const std::string& query) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize curl" << std::endl;
        return "";
    }
    
    std::string readBuffer;
    
    // Use DuckDuckGo instant answer API for free search
    char* encoded_query = curl_easy_escape(curl, query.c_str(), query.length());
    std::string url = "https://api.duckduckgo.com/?q=" + std::string(encoded_query) + "&format=json";
    curl_free(encoded_query);
    
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "User-Agent: KhushiAI/1.0");
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    
    CURLcode res = curl_easy_perform(curl);
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        return "";
    }
    
    return readBuffer;
}

std::vector<WebSearch::SearchResult> WebSearch::parse_search_results(const std::string& json_response) {
    std::vector<SearchResult> results;
    
    // Simple parsing - look for AbstractText in DuckDuckGo response
    size_t abstract_pos = json_response.find("\"AbstractText\":\"");
    if (abstract_pos != std::string::npos) {
        abstract_pos += 16; // Skip "AbstractText\":"
        size_t end_pos = json_response.find("\"", abstract_pos);
        if (end_pos != std::string::npos) {
            SearchResult result;
            result.title = "Information from DuckDuckGo";
            result.url = "https://duckduckgo.com";
            result.snippet = json_response.substr(abstract_pos, end_pos - abstract_pos);
            results.push_back(result);
        }
    }
    
    // If no abstract found, return empty results
    return results;
}

std::string WebSearch::search(const std::string& query) {
    std::cout << "Searching internet for: " << query << std::endl;
    
    std::string response = make_search_request(query);
    
    if (response.empty()) {
        return "I couldn't connect to the internet to search for information.";
    }
    
    std::vector<SearchResult> results = parse_search_results(response);
    
    if (results.empty()) {
        return "I searched the internet but couldn't find specific information about that.";
    }
    
    // Return the first result's snippet
    return results[0].snippet;
}

std::vector<WebSearch::SearchResult> WebSearch::search_results(const std::string& query) {
    std::string response = make_search_request(query);
    return parse_search_results(response);
}

} // namespace khushi
