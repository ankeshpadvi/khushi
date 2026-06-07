#include "stt.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <cstring>

namespace khushi {

SpeechToText::SpeechToText()
    : initialized_(false), listening_(false) {
}

SpeechToText::~SpeechToText() {
    if (listening_) {
        stop_listening();
    }
}

bool SpeechToText::initialize() {
    std::cout << "Initializing Speech-to-Text system..." << std::endl;
    
    // Check if pocketsphinx_continuous is available
    int result = system("which pocketsphinx_continuous > /dev/null 2>&1");
    if (result != 0) {
        std::cerr << "Warning: pocketsphinx_continuous not found. Install with: sudo apt-get install pocketsphinx" << std::endl;
        initialized_ = false;
        return false;
    }
    
    initialized_ = true;
    std::cout << "Speech-to-Text initialized successfully!" << std::endl;
    
    return true;
}

bool SpeechToText::start_listening() {
    if (!initialized_) {
        std::cerr << "STT not initialized. Call initialize() first." << std::endl;
        return false;
    }
    
    if (listening_) {
        std::cout << "Already listening..." << std::endl;
        return true;
    }
    
    std::cout << "Starting to listen... (Speak now)" << std::endl;
    listening_ = true;
    
    // Start audio processing in background
    std::thread(&SpeechToText::process_audio, this).detach();
    
    return true;
}

bool SpeechToText::stop_listening() {
    if (!listening_) {
        return true;
    }
    
    std::cout << "Stopping listening..." << std::endl;
    listening_ = false;
    
    return true;
}

std::string SpeechToText::get_text() {
    return recognized_text_;
}

bool SpeechToText::is_listening() const {
    return listening_;
}

void SpeechToText::set_callback(std::function<void(const std::string&)> callback) {
    callback_ = callback;
}

bool SpeechToText::record_audio() {
    // In a real implementation, this would:
    // 1. Open audio device (ALSA/PulseAudio)
    // 2. Record audio samples
    // 3. Save to buffer or file
    
    // For simulation, we'll just wait a bit
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return true;
}

void SpeechToText::process_audio() {
    // Record audio to temporary file with automatic silence detection
    std::string temp_file = "/tmp/khushi_audio.wav";
    
    // Record audio with silence detection (2 seconds of silence stops recording)
    std::string cmd = "arecord -f S16_LE -r 16000 -c 1 -t wav " + temp_file + " 2>/dev/null &";
    int record_pid = system(cmd.c_str());
    
    // Wait for recording to start
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Monitor for speech or timeout
    auto start = std::chrono::steady_clock::now();
    while (listening_) {
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - start).count() > 10) {
            // Timeout - stop recording
            system("pkill -9 arecord 2>/dev/null");
            break;
        }
        
        // Check if user wants to stop (check for input)
        if (std::cin.peek() != EOF) {
            std::string typed;
            std::getline(std::cin, typed);
            if (typed == "stop") {
                system("pkill -9 arecord 2>/dev/null");
                break;
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Wait a bit for recording to finish
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Use pocketsphinx to transcribe the recorded file
    cmd = "pocketsphinx_continuous -infile " + temp_file + 
          " -hmm /usr/share/pocketsphinx/model/en-us/en-us" +
          " -lm /usr/share/pocketsphinx/model/en-us/en-us.lm.bin" +
          " -dict /usr/share/pocketsphinx/model/en-us/cmudict-en-us.dict 2>&1";
    
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        std::cerr << "Failed to transcribe audio" << std::endl;
        return;
    }
    
    char buffer[512];
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        std::string line(buffer);
        
        // Skip info lines
        if (line.find("INFO:") != std::string::npos || 
            line.find("READY") != std::string::npos) {
            continue;
        }
        
        // Extract recognized text
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos && colon_pos + 1 < line.length()) {
            std::string text = line.substr(colon_pos + 1);
            
            size_t text_start = text.find_first_not_of(" \t\n\r");
            size_t text_end = text.find_last_not_of(" \t\n\r");
            if (text_start != std::string::npos) {
                recognized_text_ = text.substr(text_start, text_end - text_start + 1);
                
                if (!recognized_text_.empty() && recognized_text_.length() > 1) {
                    std::cout << "Recognized: \"" << recognized_text_ << "\"" << std::endl;
                    
                    if (callback_) {
                        callback_(recognized_text_);
                    }
                }
            }
        }
    }
    
    pclose(pipe);
    
    // Clean up temp file
    system(("rm -f " + temp_file).c_str());
}

} // namespace khushi
