#ifndef TTS_H
#define TTS_H

#include <string>

namespace khushi {

class TextToSpeech {
public:
    TextToSpeech();
    ~TextToSpeech();
    
    // Initialize text-to-speech
    bool initialize();
    
    // Speak the given text
    bool speak(const std::string& text);
    
    // Set voice properties
    void set_voice_speed(int speed);
    void set_voice_pitch(int pitch);
    
    // Stop speaking
    bool stop();
    
    // Check if currently speaking
    bool is_speaking() const;

private:
    bool initialized_;
    bool speaking_;
    int voice_speed_;
    int voice_pitch_;
    
    // System TTS command
    bool speak_with_system(const std::string& text);
};

} // namespace khushi

#endif // TTS_H
