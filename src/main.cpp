#include "llm.h"
#include "ollama_llm.h"
#include "stt.h"
#include "tts.h"
#include "command_executor.h"
#include "web_search.h"
#include "database.h"
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
    std::cout << "  help     - Show this help message\n";
    std::cout << "  quit     - Exit Khushi AI\n\n";
}

int main() {
    print_welcome();
    
    // Initialize components
    OllamaLLM llm("llama3");
    SpeechToText stt;
    TextToSpeech tts;
    CommandExecutor cmd_executor;
    WebSearch web_search;
    Database database;
    
    std::cout << "Initializing Khushi AI components...\n";
    
    if (!llm.initialize()) {
        std::cerr << "Failed to initialize Ollama LLM. Make sure Ollama is running with 'ollama serve'\n";
        return 1;
    }
    
    if (!tts.initialize()) {
        std::cerr << "Failed to initialize TTS. Continuing without audio.\n";
    }
    
    if (!stt.initialize()) {
        std::cerr << "Failed to initialize STT. Voice mode unavailable.\n";
    }
    
    // Initialize database connection
    if (!database.connect("postgresql://postgres:iBppYXLgKaAlDgwQinDRYUdoXbtEHyfU@zephyr.proxy.rlwy.net:42057/railway")) {
        std::cerr << "Failed to connect to database. Continuing without database.\n";
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
        
        // Check if user is asking for information that might need internet search
        bool needs_web_search = false;
        std::string web_info = "";
        
        // Simple keywords that might indicate need for web search
        if (input.find("search") != std::string::npos ||
            input.find("find") != std::string::npos ||
            input.find("what is") != std::string::npos ||
            input.find("who is") != std::string::npos ||
            input.find("latest") != std::string::npos ||
            input.find("news") != std::string::npos ||
            input.find("weather") != std::string::npos ||
            input.find("current") != std::string::npos) {
            needs_web_search = true;
        }
        
        std::string response;
        
        if (needs_web_search) {
            web_info = web_search.search(input);
            if (!web_info.empty() && web_info.find("couldn't") == std::string::npos) {
                // Use web search result directly
                response = "Based on internet search: " + web_info;
            } else {
                // Fall back to LLM if web search fails
                std::cout << "Khushi is thinking...\n";
                response = llm.generate_conversational_response(input);
            }
        } else {
            // Generate response using LLM with conversation context
            std::cout << "Khushi is thinking...\n";
            response = llm.generate_conversational_response(input);
        }
        
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
