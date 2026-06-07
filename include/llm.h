#ifndef LLM_H
#define LLM_H

#include <string>
#include <vector>
#include <memory>
#include <deque>

namespace khushi {

struct Message {
    std::string text;
    bool is_user;
};

class LLM {
public:
    LLM(int vocab_size = 1000, int embedding_dim = 128, int hidden_dim = 256);
    ~LLM();
    
    // Load model weights from file
    bool load_weights(const std::string& path);
    
    // Save model weights to file
    bool save_weights(const std::string& path);
    
    // Generate text from prompt with conversation context
    std::string generate(const std::string& prompt, int max_tokens = 100);
    
    // Simple training on text
    void train(const std::string& text, int epochs = 10);
    
    // Load training data from file
    bool load_training_data(const std::string& path);
    
    // Get model info
    std::string get_info() const;
    
    // Conversation memory
    void add_to_history(const std::string& text, bool is_user);
    std::string get_conversation_context() const;
    void clear_history();
    
    // Conversational response generation
    std::string generate_conversational_response(const std::string& user_input);

private:
    int vocab_size_;
    int embedding_dim_;
    int hidden_dim_;
    
    // Simple neural network weights
    std::vector<std::vector<float>> embedding_matrix_;
    std::vector<std::vector<float>> weights1_;
    std::vector<float> bias1_;
    std::vector<std::vector<float>> weights2_;
    std::vector<float> bias2_;
    
    // Tokenization
    std::vector<int> tokenize(const std::string& text);
    std::string detokenize(const std::vector<int>& tokens);
    
    // Forward pass
    std::vector<float> forward(const std::vector<int>& tokens);
    
    // Activation functions
    float relu(float x);
    float softmax(const std::vector<float>& x, int index);
    
    // Simple vocabulary
    std::vector<std::string> vocabulary_;
    void build_vocabulary(const std::string& text);
    
    // Conversation history
    std::deque<Message> conversation_history_;
    static const int MAX_HISTORY_SIZE = 10;
    
    // Pattern-based responses for common queries
    std::string get_pattern_response(const std::string& input) const;
};

} // namespace khushi

#endif // LLM_H
