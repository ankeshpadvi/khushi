#ifndef STT_H
#define STT_H

#include <string>
#include <functional>

namespace khushi {

class SpeechToText {
public:
    SpeechToText();
    ~SpeechToText();
    
    // Initialize speech recognition
    bool initialize();
    
    // Start listening for speech
    bool start_listening();
    
    // Stop listening
    bool stop_listening();
    
    // Get recognized text
    std::string get_text();
    
    // Check if currently listening
    bool is_listening() const;
    
    // Set callback for when text is recognized
    void set_callback(std::function<void(const std::string&)> callback);

private:
    bool initialized_;
    bool listening_;
    std::string recognized_text_;
    std::function<void(const std::string&)> callback_;
    
    // Audio recording
    bool record_audio();
    void process_audio();
};

} // namespace khushi

#endif // STT_H
