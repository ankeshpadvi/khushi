#include "ollama_llm.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <memory>
#include <curl/curl.h>
#include <unistd.h>

namespace khushi {

// Callback for curl to write response data
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Static initializer for curl
static bool curl_initialized = false;
static void initialize_curl() {
    if (!curl_initialized) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl_initialized = true;
    }
}

OllamaLLM::OllamaLLM(const std::string& model_name)
    : model_name_(model_name), ollama_url_("http://localhost:11434/api/generate") {
    initialize_curl();
}

OllamaLLM::~OllamaLLM() {
}

bool OllamaLLM::initialize() {
    // Check if Ollama is running
    std::string check_cmd = "curl -s http://localhost:11434/api/tags > /dev/null 2>&1";
    int result = system(check_cmd.c_str());
    
    if (result != 0) {
        std::cerr << "Ollama is not running. Start it with: ollama serve" << std::endl;
        return false;
    }
    
    std::cout << "Ollama LLM initialized with model: " << model_name_ << std::endl;
    return true;
}

std::string OllamaLLM::make_ollama_request(const std::string& prompt) {
    // Escape special characters in prompt for JSON
    std::string escaped_prompt;
    for (char c : prompt) {
        switch (c) {
            case '"': escaped_prompt += "\\\""; break;
            case '\\': escaped_prompt += "\\\\"; break;
            case '\n': escaped_prompt += "\\n"; break;
            case '\r': escaped_prompt += "\\r"; break;
            case '\t': escaped_prompt += "\\t"; break;
            default: escaped_prompt += c; break;
        }
    }
    
    // Create JSON payload
    std::string json_payload = "{\"model\":\"" + model_name_ + 
                               "\",\"prompt\":\"" + escaped_prompt + 
                               "\",\"stream\":false}";
    
    std::cout << "Sending request to Ollama..." << std::endl;
    
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize curl" << std::endl;
        return "";
    }
    
    std::string readBuffer;
    
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    
    curl_easy_setopt(curl, CURLOPT_URL, ollama_url_.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    
    CURLcode res = curl_easy_perform(curl);
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        return "";
    }
    
    std::cout << "Received response from Ollama" << std::endl;
    
    return readBuffer;
}

std::string OllamaLLM::generate(const std::string& prompt, int max_tokens) {
    std::string response = make_ollama_request(prompt);
    
    if (response.empty()) {
        return "I'm sorry, I couldn't generate a response. Please check if Ollama is running.";
    }
    
    // Parse JSON response to extract the "response" field
    // Simple parsing - look for "response":"..." pattern
    size_t response_pos = response.find("\"response\":\"");
    if (response_pos != std::string::npos) {
        response_pos += 12; // Skip "response":"
        size_t end_pos = response.find("\"", response_pos);
        if (end_pos != std::string::npos) {
            return response.substr(response_pos, end_pos - response_pos);
        }
    }
    
    return response;
}

std::string OllamaLLM::get_info() const {
    return "Ollama LLM using model: " + model_name_;
}

void OllamaLLM::add_to_history(const std::string& text, bool is_user) {
    conversation_history_.push_back({text, is_user});
    if (conversation_history_.size() > MAX_HISTORY_SIZE) {
        conversation_history_.pop_front();
    }
}

std::string OllamaLLM::get_conversation_context() const {
    std::string context;
    for (const auto& msg : conversation_history_) {
        context += (msg.is_user ? "User: " : "Assistant: ") + msg.text + "\n";
    }
    return context;
}

void OllamaLLM::clear_history() {
    conversation_history_.clear();
}

std::string OllamaLLM::generate_conversational_response(const std::string& user_input) {
    // Add user input to history
    add_to_history(user_input, true);
    
    // Build prompt with conversation context
    std::string prompt = "You are Khushi, a helpful AI assistant. Remember important information about the user.\n\n";
    
    // Add recent conversation history (last 5 messages)
    int count = 0;
    for (auto it = conversation_history_.rbegin(); it != conversation_history_.rend() && count < 5; ++it) {
        prompt += (it->is_user ? "User: " : "Assistant: ") + it->text + "\n";
        count++;
    }
    
    prompt += "User: " + user_input + "\nAssistant:";
    
    // Generate response
    std::string response = generate(prompt);
    
    // Add response to history
    if (!response.empty()) {
        add_to_history(response, false);
    }
    
    return response;
}

} // namespace khushi
