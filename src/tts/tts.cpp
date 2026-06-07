#include "tts.h"
#include <iostream>
#include <sstream>
#include <cstdlib>

namespace khushi {

TextToSpeech::TextToSpeech()
    : initialized_(false), speaking_(false), voice_speed_(150), voice_pitch_(50) {
}

TextToSpeech::~TextToSpeech() {
    if (speaking_) {
        stop();
    }
}

bool TextToSpeech::initialize() {
    std::cout << "Initializing Text-to-Speech system..." << std::endl;
    
    // Check if espeak is available
    int result = system("which espeak > /dev/null 2>&1");
    if (result != 0) {
        std::cerr << "Warning: espeak not found. Install with: sudo apt-get install espeak" << std::endl;
        std::cerr << "Falling back to system TTS..." << std::endl;
        initialized_ = false;
        return false;
    }
    
    initialized_ = true;
    std::cout << "Text-to-Speech initialized successfully!" << std::endl;
    
    return true;
}

bool TextToSpeech::speak(const std::string& text) {
    if (!initialized_) {
        std::cerr << "TTS not initialized. Call initialize() first." << std::endl;
        return false;
    }
    
    if (text.empty()) {
        return true;
    }
    
    speaking_ = true;
    std::cout << "Khushi: \"" << text << "\"" << std::endl;
    
    bool success = speak_with_system(text);
    
    speaking_ = false;
    return success;
}

bool TextToSpeech::speak_with_system(const std::string& text) {
    // Try espeak with female voice first
    std::ostringstream cmd;
    cmd << "espeak -ven+f3 -s " << voice_speed_ << " -p " << voice_pitch_ 
        << " \"" << text << "\" 2>/dev/null";
    
    int result = system(cmd.str().c_str());
    
    // If espeak fails, try other methods
    if (result != 0) {
        // Try festival
        cmd.str("");
        cmd.clear();
        cmd << "echo \"" << text << "\" | festival --tts 2>/dev/null";
        result = system(cmd.str().c_str());
    }
    
    if (result != 0) {
        // Last resort: print to console (already done above)
        std::cout << "(Audio not available - text displayed above)" << std::endl;
        return false;
    }
    
    return true;
}

void TextToSpeech::set_voice_speed(int speed) {
    voice_speed_ = speed;
}

void TextToSpeech::set_voice_pitch(int pitch) {
    voice_pitch_ = pitch;
}

bool TextToSpeech::stop() {
    speaking_ = false;
    
    // Kill any running espeak process
    system("pkill -9 espeak 2>/dev/null");
    system("pkill -9 festival 2>/dev/null");
    
    return true;
}

bool TextToSpeech::is_speaking() const {
    return speaking_;
}

} // namespace khushi
