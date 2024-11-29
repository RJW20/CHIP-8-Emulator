cmake_minimum_required(VERSION 3.10)
project(CHIP_8_Emulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add SDL2
include_directories(third_party/SDL2/include)
link_directories(third_party/SDL2/lib)

# Add source files
set(SOURCE_FILES src/main.cpp)
add_executable(chip8 ${SOURCE_FILES})

# Link SDL2
target_link_libraries(Snake SDL2 SDL2main)