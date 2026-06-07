#include "llm.h"
#include "stt.h"
#include "tts.h"
#include "command_executor.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

using namespace khushi;

void print_welcome() {
    std::cout << "\n========================================\n";
    std::cout << "           Khushi AI v1.0              \n";
    std::cout << "========================================\n";
    std::cout << "Hello! I am Khushi, your personal AI assistant.\n";
    std::cout << "I can listen to you and respond with speech.\n";
    std::cout << "Type 'quit' to exit or 'voice' to use voice mode.\n";
    std::cout << "========================================\n\n";
}

void print_help() {
    std::cout << "\nCommands:\n";
    std::cout << "  voice    - Switch to voice interaction mode\n";
    std::cout << "  text     - Switch to text interaction mode\n";
    std::cout << "  train    - Train the LLM with sample text\n";
    std::cout <<  "  save     - Save model weights\n";
    std::cout << "  load     - Load model weights\n";
    std::cout << "  help     - Show this help message\n";
    std::cout << "  quit     - Exit Khushi AI\n\n";
}

int main() {
    print_welcome();
    
    // Initialize components
    LLM llm(1000, 128, 256);
    SpeechToText stt;
    TextToSpeech tts;
    CommandExecutor cmd_executor;
    
    std::cout << "Initializing Khushi AI components...\n";
    
    if (!tts.initialize()) {
        std::cerr << "Failed to initialize TTS. Continuing without audio.\n";
    }
    
    if (!stt.initialize()) {
        std::cerr << "Failed to initialize STT. Voice mode unavailable.\n";
    }
    
    // Set up permission callback for command executor
    cmd_executor.set_permission_callback([](const std::string& cmd) {
        std::cout << "\n[Security] Execute command: " << cmd << "? (y/n): ";
        std::string response;
        std::getline(std::cin, response);
        return response == "y" || response == "yes";
    });
    
    std::cout << "LLM Info: " << llm.get_info() << "\n\n";
    
    // Greet the user
    std::string greeting = "Hello! I am Khushi, your personal AI assistant. How can I help you today?";
    std::cout << "Khushi: " << greeting << "\n";
    tts.speak(greeting);
    
    bool voice_mode = false;
    std::string input;
    
    while (true) {
        if (voice_mode) {
            std::cout << "\n[Voice Mode - Listening...]\n";
            std::cout << "Press Enter when you're done speaking...\n";
            
            stt.start_listening();
            std::getline(std::cin, input);
            stt.stop_listening();
            
            input = stt.get_text();
            if (input.empty()) {
                std::cout << "No speech detected. Please try again.\n";
                continue;
            }
        } else {
            std::cout << "\nYou: ";
            std::getline(std::cin, input);
        }
        
        // Trim input
        size_t start = input.find_first_not_of(" \t\n\r");
        size_t end = input.find_last_not_of(" \t\n\r");
        if (start != std::string::npos) {
            input = input.substr(start, end - start + 1);
        }
        
        if (input.empty()) {
            continue;
        }
        
        // Check for commands
        if (input == "quit" || input == "exit" || input == "bye") {
            std::string goodbye = "Goodbye! It was nice talking to you.";
            std::cout << "Khushi: " << goodbye << "\n";
            tts.speak(goodbye);
            break;
        }
        
        if (input == "voice") {
            voice_mode = true;
            std::cout << "Switched to continuous voice mode.\n";
            tts.speak("Voice mode activated. I'm listening now.");
            continue;
        }
        
        if (input == "text") {
            voice_mode = false;
            std::cout << "Switched to text mode.\n";
            tts.speak("Text mode activated.");
            continue;
        }
        
        if (input == "help") {
            print_help();
            continue;
        }
        
        if (input == "train") {
            std::cout << "Training LLM with sample conversations...\n";
            std::string training_text = "hello how are you i am fine thank you what is your name my name is khushi i am an ai assistant how can i help you please help me with my work i will do my best to help you";
            llm.train(training_text, 100);
            std::cout << "Training completed!\n";
            tts.speak("Training completed. I'm smarter now!");
            continue;
        }
        
        if (input == "save") {
            std::string path = "models/khushi_model.bin";
            if (llm.save_weights(path)) {
                std::cout << "Model saved to " << path << "\n";
                tts.speak("Model saved successfully.");
            } else {
                std::cout << "Failed to save model.\n";
            }
            continue;
        }
        
        if (input == "load") {
            std::string path = "models/khushi_model.bin";
            if (llm.load_weights(path)) {
                std::cout << "Model loaded from " << path << "\n";
                tts.speak("Model loaded successfully.");
            } else {
                std::cout << "Failed to load model. Using default weights.\n";
            }
            continue;
        }
        
        // Generate response using LLM with conversation context
        std::cout << "Khushi is thinking...\n";
        std::string response = llm.generate_conversational_response(input);
        
        // Check if input is a command
        CommandResult cmd_result = cmd_executor.execute_natural_command(input);
        if (cmd_result.success) {
            response = cmd_result.output;
            if (!cmd_result.error.empty()) {
                response += " (Note: " + cmd_result.error + ")";
            }
        }
        
        // Speak and display response
        std::cout << "Khushi: " << response << "\n";
        tts.speak(response);
        
        // In continuous voice mode, automatically continue to listening
        if (voice_mode) {
            continue;
        }
    }
    
    std::cout << "\nThank you for using Khushi AI!\n";
    return 0;
}
