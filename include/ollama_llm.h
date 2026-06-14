#ifndef OLLAMA_LLM_H
#define OLLAMA_LLM_H

#include <string>
#include <vector>
#include <memory>
#include <deque>
#include "llm.h"

namespace khushi {

class OllamaLLM {
public:
    OllamaLLM(const std::string& model_name = "llama3");
    ~OllamaLLM();
    
    // Initialize Ollama connection
    bool initialize();
    
    // Generate text from prompt
    std::string generate(const std::string& prompt, int max_tokens = 100);
    
    // Get model info
    std::string get_info() const;
    
    // Conversation memory
    void add_to_history(const std::string& text, bool is_user);
    std::string get_conversation_context() const;
    void clear_history();
    
    // Conversational response generation
    std::string generate_conversational_response(const std::string& user_input);

private:
    std::string model_name_;
    std::string ollama_url_;
    
    // Conversation history
    std::deque<Message> conversation_history_;
    static const int MAX_HISTORY_SIZE = 10;
    
    // Make HTTP request to Ollama API
    std::string make_ollama_request(const std::string& prompt);
};

} // namespace khushi

#endif // OLLAMA_LLM_H
