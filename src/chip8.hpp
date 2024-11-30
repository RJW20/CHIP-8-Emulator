#ifndef CHIP_8_HPP
#define CHIP_8_HPP

#include <cstdint>
#include <string>
#include <stack>
#include <SDL2/SDL.h>

class Chip8 {
public:
    Chip8(const std::string rom_path);
    ~Chip8();

    void run();

private:
    std::string rom_path;       // ROM location

    SDL_Window* window;         // SDL window
    uint8_t display[64*32];     // Display buffer
    uint8_t keys[16];           // Keypad

    uint8_t memory[4096];       // Memory
    uint16_t pc;                // Program counter
    uint16_t I;                 // Index register
    std::stack<uint8_t> stack;  // Call stack
    uint8_t V[16];              // Variable registers

    uint8_t delay;              // Delay timer
    uint8_t sound;              // Sound timer

    void hard_reset();
    void soft_reset();

    bool handle_events();
    void advance();
    //void render();
};

#endif