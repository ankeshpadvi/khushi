#include "llm.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <random>
#include <cmath>
#include <algorithm>

namespace khushi {

LLM::LLM(int vocab_size, int embedding_dim, int hidden_dim)
    : vocab_size_(vocab_size), embedding_dim_(embedding_dim), hidden_dim_(hidden_dim) {
    
    // Initialize weights with random values
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<float> dist(0.0f, 0.1f);
    
    // Initialize embedding matrix
    embedding_matrix_.resize(vocab_size_);
    for (int i = 0; i < vocab_size_; ++i) {
        embedding_matrix_[i].resize(embedding_dim_);
        for (int j = 0; j < embedding_dim_; ++j) {
            embedding_matrix_[i][j] = dist(gen);
        }
    }
    
    // Initialize first layer weights
    weights1_.resize(embedding_dim_);
    for (int i = 0; i < embedding_dim_; ++i) {
        weights1_[i].resize(hidden_dim_);
        for (int j = 0; j < hidden_dim_; ++j) {
            weights1_[i][j] = dist(gen);
        }
    }
    
    bias1_.resize(hidden_dim_);
    for (int i = 0; i < hidden_dim_; ++i) {
        bias1_[i] = dist(gen);
    }
    
    // Initialize second layer weights
    weights2_.resize(hidden_dim_);
    for (int i = 0; i < hidden_dim_; ++i) {
        weights2_[i].resize(vocab_size_);
        for (int j = 0; j < vocab_size_; ++j) {
            weights2_[i][j] = dist(gen);
        }
    }
    
    bias2_.resize(vocab_size_);
    for (int i = 0; i < vocab_size_; ++i) {
        bias2_[i] = dist(gen);
    }
    
    // Build expanded vocabulary with diverse words
    vocabulary_ = {
        // Greetings and basic
        "hello", "hi", "hey", "greetings", "welcome", "how", "are", "you", "i", "am",
        "fine", "good", "great", "okay", "alright", "sure", "yes", "no", "maybe",
        
        // Personal pronouns and basic verbs
        "what", "is", "your", "name", "khushi", "ai", "help", "me", "please",
        "thank", "thanks", "bye", "goodbye", "can", "do", "the", "a", "an", "to",
        "for", "with", "and", "or", "but", "not", "this", "that", "it", "was",
        "will", "be", "have", "has", "had", "my", "your", "his", "her", "our",
        
        // Communication verbs
        "think", "feel", "know", "understand", "tell", "say", "speak", "talk",
        "listen", "hear", "see", "look", "watch", "want", "need", "like", "love",
        "ask", "answer", "question", "reply", "respond", "explain", "describe",
        
        // Emotions
        "happy", "sad", "angry", "excited", "bored", "tired", "great", "awesome",
        "wonderful", "amazing", "terrible", "awful", "scared", "afraid", "worried",
        "calm", "peaceful", "joyful", "cheerful", "depressed", "anxious", "nervous",
        "confident", "proud", "ashamed", "guilty", "jealous", "envious", "grateful",
        
        // Adverbs and connectors
        "really", "very", "quite", "just", "only", "also", "too", "well", "then",
        "because", "why", "when", "where", "who", "which", "how", "much", "many",
        "always", "never", "sometimes", "often", "usually", "rarely", "already",
        "still", "yet", "again", "once", "twice", "before", "after", "during",
        
        // Time expressions
        "time", "day", "today", "tomorrow", "yesterday", "now", "later", "soon",
        "morning", "afternoon", "evening", "night", "week", "month", "year",
        "hour", "minute", "second", "past", "present", "future", "early", "late",
        
        // Politeness
        "please", "sorry", "excuse", "thanks", "welcome", "appreciate", "regret",
        
        // Actions and activities
        "go", "come", "walk", "run", "jump", "sit", "stand", "lie", "sleep",
        "eat", "drink", "cook", "read", "write", "learn", "teach", "study",
        "work", "play", "create", "make", "build", "fix", "break", "buy", "sell",
        "give", "take", "get", "receive", "send", "bring", "carry", "hold",
        
        // Objects and things
        "thing", "stuff", "item", "object", "tool", "device", "machine", "computer",
        "phone", "book", "paper", "pen", "pencil", "table", "chair", "desk",
        "room", "house", "home", "building", "car", "bus", "train", "plane",
        "food", "water", "drink", "meal", "breakfast", "lunch", "dinner",
        
        // Concepts and abstract
        "idea", "thought", "concept", "theory", "fact", "truth", "lie", "story",
        "problem", "solution", "answer", "question", "reason", "cause", "effect",
        "result", "outcome", "goal", "purpose", "meaning", "value", "importance",
        "success", "failure", "mistake", "error", "correct", "wrong", "right",
        
        // People and relationships
        "person", "people", "man", "woman", "child", "boy", "girl", "family",
        "friend", "enemy", "stranger", "neighbor", "colleague", "partner", "team",
        "group", "community", "society", "world", "country", "city", "town",
        
        // Nature and environment
        "sun", "moon", "star", "sky", "cloud", "rain", "snow", "wind", "storm",
        "tree", "flower", "plant", "grass", "mountain", "hill", "river", "lake",
        "ocean", "sea", "beach", "forest", "animal", "bird", "fish", "dog", "cat",
        
        // Colors and descriptions
        "red", "blue", "green", "yellow", "orange", "purple", "pink", "black",
        "white", "gray", "brown", "big", "small", "large", "tiny", "huge",
        "long", "short", "tall", "wide", "narrow", "thick", "thin", "heavy",
        "light", "dark", "bright", "dim", "hot", "cold", "warm", "cool",
        
        // Numbers and quantities
        "one", "two", "three", "four", "five", "six", "seven", "eight", "nine",
        "ten", "hundred", "thousand", "million", "billion", "first", "second",
        "third", "last", "next", "previous", "all", "some", "any", "none", "every",
        
        // Technology and modern terms
        "internet", "web", "site", "app", "software", "hardware", "data", "code",
        "program", "system", "network", "server", "client", "user", "account",
        "password", "login", "logout", "email", "message", "chat", "call", "video",
        
        // Health and body
        "health", "body", "mind", "brain", "heart", "hand", "foot", "head",
        "eye", "ear", "nose", "mouth", "face", "skin", "hair", "sick", "ill",
        "pain", "hurt", "medicine", "doctor", "hospital", "clinic", "treatment",
        
        // Education and knowledge
        "school", "university", "college", "class", "lesson", "subject", "topic",
        "skill", "ability", "talent", "gift", "knowledge", "wisdom", "intelligence",
        "smart", "clever", "wise", "stupid", "dumb", "foolish", "smart",
        
        // Business and work
        "business", "company", "job", "work", "career", "profession", "office",
        "manager", "boss", "employee", "worker", "salary", "money", "pay", "cost",
        "price", "expensive", "cheap", "free", "buy", "sell", "trade", "market",
        
        // Entertainment and media
        "movie", "film", "music", "song", "game", "sport", "play", "show", "tv",
        "radio", "news", "media", "art", "painting", "drawing", "photo", "picture",
        "fun", "funny", "joke", "laugh", "smile", "entertainment", "hobby",
        
        // Directions and locations
        "here", "there", "where", "up", "down", "left", "right", "north", "south",
        "east", "west", "inside", "outside", "front", "back", "top", "bottom",
        "center", "middle", "corner", "edge", "side", "direction", "way", "path",
        
        // Quality and condition
        "better", "best", "worse", "worst", "good", "bad", "excellent", "poor",
        "perfect", "imperfect", "complete", "incomplete", "full", "empty", "clean",
        "dirty", "new", "old", "young", "fresh", "stale", "rich", "poor",
        
        // Questions and inquiry
        "who", "what", "when", "where", "why", "how", "which", "whose", "whom",
        "anything", "something", "nothing", "everything", "someone", "anyone",
        "everyone", "noone", "somewhere", "anywhere", "everywhere", "nowhere",
        
        // Possibility and certainty
        "possible", "impossible", "probable", "likely", "unlikely", "certain",
        "uncertain", "sure", "doubt", "believe", "trust", "hope", "wish", "dream",
        "expect", "predict", "guess", "assume", "suppose", "imagine"
    };
    
    // Fill remaining vocabulary with tokens
    while (vocabulary_.size() < vocab_size_) {
        vocabulary_.push_back("<token_" + std::to_string(vocabulary_.size()) + ">");
    }
}

LLM::~LLM() {}

bool LLM::load_weights(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open weights file: " << path << std::endl;
        return false;
    }
    
    // Load embedding matrix
    for (int i = 0; i < vocab_size_; ++i) {
        for (int j = 0; j < embedding_dim_; ++j) {
            file.read(reinterpret_cast<char*>(&embedding_matrix_[i][j]), sizeof(float));
        }
    }
    
    // Load weights1
    for (int i = 0; i < embedding_dim_; ++i) {
        for (int j = 0; j < hidden_dim_; ++j) {
            file.read(reinterpret_cast<char*>(&weights1_[i][j]), sizeof(float));
        }
    }
    
    // Load bias1
    for (int i = 0; i < hidden_dim_; ++i) {
        file.read(reinterpret_cast<char*>(&bias1_[i]), sizeof(float));
    }
    
    // Load weights2
    for (int i = 0; i < hidden_dim_; ++i) {
        for (int j = 0; j < vocab_size_; ++j) {
            file.read(reinterpret_cast<char*>(&weights2_[i][j]), sizeof(float));
        }
    }
    
    // Load bias2
    for (int i = 0; i < vocab_size_; ++i) {
        file.read(reinterpret_cast<char*>(&bias2_[i]), sizeof(float));
    }
    
    file.close();
    return true;
}

bool LLM::save_weights(const std::string& path) {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to create weights file: " << path << std::endl;
        return false;
    }
    
    // Save embedding matrix
    for (int i = 0; i < vocab_size_; ++i) {
        for (int j = 0; j < embedding_dim_; ++j) {
            file.write(reinterpret_cast<const char*>(&embedding_matrix_[i][j]), sizeof(float));
        }
    }
    
    // Save weights1
    for (int i = 0; i < embedding_dim_; ++i) {
        for (int j = 0; j < hidden_dim_; ++j) {
            file.write(reinterpret_cast<const char*>(&weights1_[i][j]), sizeof(float));
        }
    }
    
    // Save bias1
    for (int i = 0; i < hidden_dim_; ++i) {
        file.write(reinterpret_cast<const char*>(&bias1_[i]), sizeof(float));
    }
    
    // Save weights2
    for (int i = 0; i < hidden_dim_; ++i) {
        for (int j = 0; j < vocab_size_; ++j) {
            file.write(reinterpret_cast<const char*>(&weights2_[i][j]), sizeof(float));
        }
    }
    
    // Save bias2
    for (int i = 0; i < vocab_size_; ++i) {
        file.write(reinterpret_cast<const char*>(&bias2_[i]), sizeof(float));
    }
    
    file.close();
    return true;
}

std::vector<int> LLM::tokenize(const std::string& text) {
    std::vector<int> tokens;
    std::string word;
    std::istringstream iss(text);
    
    while (iss >> word) {
        // Convert to lowercase
        std::transform(word.begin(), word.end(), word.begin(), ::tolower);
        
        // Find in vocabulary
        auto it = std::find(vocabulary_.begin(), vocabulary_.end(), word);
        if (it != vocabulary_.end()) {
            tokens.push_back(std::distance(vocabulary_.begin(), it));
        } else {
            // Use unknown token
            tokens.push_back(0);
        }
    }
    
    return tokens;
}

std::string LLM::detokenize(const std::vector<int>& tokens) {
    std::string result;
    for (int token : tokens) {
        if (token >= 0 && token < vocabulary_.size()) {
            result += vocabulary_[token] + " ";
        }
    }
    return result;
}

float LLM::relu(float x) {
    return std::max(0.0f, x);
}

float LLM::softmax(const std::vector<float>& x, int index) {
    float max_val = *std::max_element(x.begin(), x.end());
    float sum = 0.0f;
    
    for (float val : x) {
        sum += std::exp(val - max_val);
    }
    
    return std::exp(x[index] - max_val) / sum;
}

std::vector<float> LLM::forward(const std::vector<int>& tokens) {
    if (tokens.empty()) {
        return std::vector<float>(vocab_size_, 0.0f);
    }
    
    // Average embeddings
    std::vector<float> avg_embedding(embedding_dim_, 0.0f);
    for (int token : tokens) {
        if (token >= 0 && token < vocab_size_) {
            for (int j = 0; j < embedding_dim_; ++j) {
                avg_embedding[j] += embedding_matrix_[token][j];
            }
        }
    }
    
    for (int j = 0; j < embedding_dim_; ++j) {
        avg_embedding[j] /= tokens.size();
    }
    
    // First layer
    std::vector<float> hidden(hidden_dim_, 0.0f);
    for (int i = 0; i < hidden_dim_; ++i) {
        hidden[i] = bias1_[i];
        for (int j = 0; j < embedding_dim_; ++j) {
            hidden[i] += avg_embedding[j] * weights1_[j][i];
        }
        hidden[i] = relu(hidden[i]);
    }
    
    // Second layer (output)
    std::vector<float> output(vocab_size_, 0.0f);
    for (int i = 0; i < vocab_size_; ++i) {
        output[i] = bias2_[i];
        for (int j = 0; j < hidden_dim_; ++j) {
            output[i] += hidden[j] * weights2_[j][i];
        }
    }
    
    return output;
}

std::string LLM::generate(const std::string& prompt, int max_tokens) {
    auto tokens = tokenize(prompt);
    std::string result = prompt + " ";
    
    std::random_device rd;
    std::mt19937 gen(rd());
    
    for (int i = 0; i < max_tokens; ++i) {
        auto output = forward(tokens);
        
        // Sample from output distribution
        std::vector<float> probs(vocab_size_);
        for (int j = 0; j < vocab_size_; ++j) {
            probs[j] = softmax(output, j);
        }
        
        std::discrete_distribution<int> dist(probs.begin(), probs.end());
        int next_token = dist(gen);
        
        if (next_token < vocabulary_.size()) {
            result += vocabulary_[next_token] + " ";
            tokens.push_back(next_token);
        }
        
        // Simple stopping condition
        if (vocabulary_[next_token] == "bye" || vocabulary_[next_token] == "goodbye") {
            break;
        }
    }
    
    return result;
}

void LLM::train(const std::string& text, int epochs) {
    build_vocabulary(text);
    
    auto tokens = tokenize(text);
    if (tokens.size() < 2) return;
    
    float learning_rate = 0.01f;
    
    // Improved training: simple gradient descent for next-token prediction
    for (int epoch = 0; epoch < epochs; ++epoch) {
        float total_loss = 0.0f;
        
        // Train on sequences of tokens
        for (size_t i = 0; i < tokens.size() - 1; ++i) {
            // Create input sequence (context window of 3 tokens)
            std::vector<int> context;
            for (size_t j = std::max(0, (int)i - 2); j <= i; ++j) {
                if (j < tokens.size()) {
                    context.push_back(tokens[j]);
                }
            }
            
            // Forward pass
            auto output = forward(context);
            
            // Target is the next token
            int target_token = tokens[i + 1];
            
            // Calculate loss and gradients (simplified)
            std::vector<float> target_probs(vocab_size_, 0.0f);
            if (target_token >= 0 && target_token < vocab_size_) {
                target_probs[target_token] = 1.0f;
            }
            
            // Compute gradients for output layer
            std::vector<float> output_grad(vocab_size_);
            for (int j = 0; j < vocab_size_; ++j) {
                float prob = softmax(output, j);
                output_grad[j] = prob - target_probs[j];
                total_loss += (target_probs[j] - prob) * (target_probs[j] - prob);
            }
            
            // Backpropagate to weights2 and bias2
            for (int j = 0; j < hidden_dim_; ++j) {
                for (int k = 0; k < vocab_size_; ++k) {
                    weights2_[j][k] -= learning_rate * output_grad[k] * 0.1f; // Simplified gradient
                }
            }
            
            for (int k = 0; k < vocab_size_; ++k) {
                bias2_[k] -= learning_rate * output_grad[k];
            }
            
            // Backpropagate to hidden layer (simplified)
            std::vector<float> hidden_grad(hidden_dim_, 0.0f);
            for (int j = 0; j < hidden_dim_; ++j) {
                for (int k = 0; k < vocab_size_; ++k) {
                    hidden_grad[j] += output_grad[k] * weights2_[j][k];
                }
            }
            
            // Update weights1 and bias1
            for (int j = 0; j < embedding_dim_; ++j) {
                for (int k = 0; k < hidden_dim_; ++k) {
                    weights1_[j][k] -= learning_rate * hidden_grad[k] * 0.01f;
                }
            }
            
            for (int k = 0; k < hidden_dim_; ++k) {
                bias1_[k] -= learning_rate * hidden_grad[k];
            }
        }
        
        // Print progress every 10 epochs
        if ((epoch + 1) % 10 == 0) {
            std::cout << "Epoch " << (epoch + 1) << "/" << epochs 
                      << ", Loss: " << (total_loss / tokens.size()) << std::endl;
        }
    }
}

bool LLM::load_training_data(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open training data file: " << path << std::endl;
        return false;
    }
    
    std::string content;
    std::string line;
    
    while (std::getline(file, line)) {
        // Skip comment lines (starting with #)
        if (!line.empty() && line[0] != '#') {
            content += line + " ";
        }
    }
    
    file.close();
    
    if (content.empty()) {
        std::cerr << "No training data found in file: " << path << std::endl;
        return false;
    }
    
    // Build vocabulary from training data
    build_vocabulary(content);
    
    // Train on the loaded data
    train(content, 20);
    
    std::cout << "Loaded and trained on " << content.length() 
              << " characters of training data from " << path << std::endl;
    
    return true;
}

void LLM::build_vocabulary(const std::string& text) {
    std::istringstream iss(text);
    std::string word;
    
    while (iss >> word) {
        std::transform(word.begin(), word.end(), word.begin(), ::tolower);
        
        if (std::find(vocabulary_.begin(), vocabulary_.end(), word) == vocabulary_.end()) {
            if (vocabulary_.size() < vocab_size_) {
                vocabulary_.push_back(word);
            }
        }
    }
}

std::string LLM::get_info() const {
    return "Khushi LLM - Vocab: " + std::to_string(vocab_size_) +
           ", Embedding: " + std::to_string(embedding_dim_) +
           ", Hidden: " + std::to_string(hidden_dim_);
}

void LLM::add_to_history(const std::string& text, bool is_user) {
    conversation_history_.push_back({text, is_user});
    if (conversation_history_.size() > MAX_HISTORY_SIZE) {
        conversation_history_.pop_front();
    }
}

std::string LLM::get_conversation_context() const {
    std::string context;
    for (const auto& msg : conversation_history_) {
        context += (msg.is_user ? "User: " : "Khushi: ") + msg.text + "\n";
    }
    return context;
}

void LLM::clear_history() {
    conversation_history_.clear();
}

std::string LLM::get_pattern_response(const std::string& input) const {
    std::string lower_input = input;
    std::transform(lower_input.begin(), lower_input.end(), lower_input.begin(), ::tolower);
    
    // Greeting patterns
    if (lower_input.find("hello") != std::string::npos || 
        lower_input.find("hi") != std::string::npos ||
        lower_input.find("hey") != std::string::npos) {
        return "Hello! How can I help you today?";
    }
    
    // How are you
    if (lower_input.find("how are you") != std::string::npos) {
        return "I'm doing great, thank you for asking! How about you?";
    }
    
    // What's your name
    if (lower_input.find("your name") != std::string::npos) {
        return "My name is Khushi, I'm your personal AI assistant.";
    }
    
    // Thank you
    if (lower_input.find("thank") != std::string::npos) {
        return "You're welcome! Is there anything else I can help with?";
    }
    
    // Goodbye
    if (lower_input.find("bye") != std::string::npos || 
        lower_input.find("goodbye") != std::string::npos) {
        return "Goodbye! It was nice talking to you.";
    }
    
    // Help
    if (lower_input.find("help") != std::string::npos) {
        return "I'm here to help! You can ask me questions, have a conversation, or give me commands.";
    }
    
    // Who are you
    if (lower_input.find("who are you") != std::string::npos) {
        return "I'm Khushi, an AI assistant designed to help you with various tasks and have conversations.";
    }
    
    // What can you do
    if (lower_input.find("what can you do") != std::string::npos) {
        return "I can listen to you, speak with you, answer questions, and help with various tasks. Just ask!";
    }
    
    // I love you
    if (lower_input.find("love") != std::string::npos) {
        return "That's very kind of you! I enjoy our conversations too.";
    }
    
    // Are you real
    if (lower_input.find("real") != std::string::npos) {
        return "I'm a real AI assistant, here to help you in any way I can.";
    }
    
    return ""; // No pattern match
}

std::string LLM::generate_conversational_response(const std::string& user_input) {
    // First check for pattern-based responses
    std::string pattern_response = get_pattern_response(user_input);
    if (!pattern_response.empty()) {
        add_to_history(user_input, true);
        add_to_history(pattern_response, false);
        return pattern_response;
    }
    
    // Add user input to history
    add_to_history(user_input, true);
    
    // Get conversation context
    std::string context = get_conversation_context();
    
    // Generate response using the LLM with context
    std::string prompt = context + "User: " + user_input + "\nKhushi:";
    std::string response = generate(prompt, 30);
    
    // Clean up response
    size_t start = response.find_last_of("Khushi:");
    if (start != std::string::npos && start + 8 < response.size()) {
        response = response.substr(start + 8);
    }
    
    // Trim whitespace
    start = response.find_first_not_of(" \t\n\r");
    size_t end = response.find_last_not_of(" \t\n\r");
    if (start != std::string::npos && end != std::string::npos && end >= start && end < response.size()) {
        response = response.substr(start, end - start + 1);
    }
    
    // If response is too short or empty, use a default
    if (response.length() < 3) {
        response = "I understand. Tell me more about that.";
    }
    
    // Add response to history
    add_to_history(response, false);
    
    return response;
}

} // namespace khushi
