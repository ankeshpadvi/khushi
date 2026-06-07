# Khushi AI

Khushi AI is a personal AI assistant that can listen and speak with you, featuring its own LLM implemented in C/C++.

## Features

- **Speech Recognition**: Listen to your voice commands
- **Text-to-Speech**: Speak responses back to you
- **Custom LLM**: Lightweight language model in C++
- **Real-time Interaction**: Conversational AI experience

## Project Structure

```
khushi/
├── src/
│   ├── llm/           # LLM implementation
│   ├── stt/           # Speech-to-Text module
│   ├── tts/           # Text-to-Speech module
│   └── main.cpp       # Main integration
├── include/           # Header files
├── models/            # Model weights
└── CMakeLists.txt     # Build system
```

## Dependencies

- C++17 or higher
- CMake 3.10+
- ALSA/PulseAudio (for audio)
- espeak-ng (for TTS)
- PocketSphinx (for STT)

## Building

```bash
mkdir build && cd build
cmake ..
make
```

## Running

```bash
./khushi_ai
```

## License

MIT License