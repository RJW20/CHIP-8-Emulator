#ifndef CHIP_8_HPP
#define CHIP_8_HPP

#include <cstdint>

class Chip8 {
public:
    Chip8();
    ~Chip8();

    void run();

private:
    uint16_t pc;            // Program counter
    uint16_t I;             // Index register

    uint8_t memory[4096];   // Memory
    uint8_t V[16];          // Variable registers

    uint16_t stack[16];     // Stack
    uint8_t sp;             // Stack pointer

    uint8_t delay;          // Delay timer
    uint8_t sound;          // Sound timer

    void handle_events();
    void update();
    void render();

};

#endif