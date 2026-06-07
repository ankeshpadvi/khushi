cmake_minimum_required(VERSION 3.10)
project(KhushiAI VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Source files
set(LLM_SOURCES
    src/llm/llm.cpp
)

set(STT_SOURCES
    src/stt/stt.cpp
)

set(TTS_SOURCES
    src/tts/tts.cpp
)

set(COMMAND_SOURCES
    src/command_executor/command_executor.cpp
)

set(MAIN_SOURCES
    src/main.cpp
)

# Create library for LLM
add_library(llm_lib STATIC ${LLM_SOURCES})

# Create library for STT
add_library(stt_lib STATIC ${STT_SOURCES})

# Create library for TTS
add_library(tts_lib STATIC ${TTS_SOURCES})

# Create library for Command Executor
add_library(command_lib STATIC ${COMMAND_SOURCES})

# Create executable
add_executable(khushi_ai ${MAIN_SOURCES})

# Link libraries
target_link_libraries(khushi_ai
    llm_lib
    stt_lib
    tts_lib
    command_lib
    pthread
)

# Installation
install(TARGETS khushi_ai DESTINATION bin)
install(DIRECTORY models DESTINATION share/khushi_ai)

# Print build information
message(STATUS "Building Khushi AI v${PROJECT_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
