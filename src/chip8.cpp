#include <iostream>
#include <cstdint>
#include <string>
#include <fstream>
#include <chrono>
#include "chip8.hpp"

// Font
unsigned char fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};

/* Initialise the Graphics, load the ROM into memory and set all components to
their initial values. */
Chip8::Chip8(std::string file_path) {

    // Initialise the graphics
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Failed to load SDL2 due to error:" << SDL_GetError() <<
            std::endl;
        exit(1);
    };
    window = SDL_CreateWindow(
        "CHIP-8 Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        64, 32, 0
    );

    rom_path = file_path;
    hard_reset();
}

Chip8::~Chip8() {
    SDL_DestroyWindow(window);
    SDL_Quit();
}

// Load the ROM into memory and soft reset
void Chip8::hard_reset() {

    std::cout << "Loading ROM: " << rom_path << std::endl;

    // Open the file in binary mode
    std::ifstream file(rom_path, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Failed to open ROM: " << rom_path << std::endl;
        exit(2);
    }

    // Get the file size
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    if (size > 4096 - 512) {
        std::cerr << "ROM is too large!" << std::endl;
    }

    // Read the file into a buffer
    char* buffer = new char[size];
    if (!file.read(buffer, size)) {
        std::cerr << "Failed to read file!" << std::endl;
        exit(3);
    }
    file.close();

    // Fill the memory
    for (int i = 0; i < 80; ++i) {
        memory[i] = fontset[i];
    }
    for (int i = 80; i < 512; ++i) {
        memory[i] = 0;
    }
    for (int i = 0; i < size; ++i) {
        memory[i + 512] = (uint8_t) buffer[i];
    }
    for (int i = 512 + size; i < 4096; ++i) {
        memory[i] = 0;
    }
    
    soft_reset();
}

// Set all components to their initial values
void Chip8::soft_reset() {

    for (int i = 0; i < 64 * 32; ++i) {
        display[i] = 0;
    }
    for (int i = 0; i < 16; ++i) {
        keys[i] = 0;
        V[i] = 0;
    }

    pc = 0x200;
    I = 0;

    delay = 0;
    sound = 0;

    std::srand(static_cast<unsigned>(std::time(0)));
}

// Main game loop
void Chip8::run() {

    std::cout << keys[0] << std::endl;
    
    while(true) {

        if (!handle_events()) {
            break;
        }

    }
}

// Keypress and quit handling, returns false when window is quit
bool Chip8::handle_events() {

    SDL_Event event;
    while (SDL_PollEvent(&event)) {

        if (event.type == SDL_QUIT) {
            return false;
        }

        // Process keydown events
        if (event.type == SDL_KEYDOWN) {
            switch(event.key.keysym.sym) {
                case SDLK_F1:
                    hard_reset();
                    break;
                case SDLK_F2:
                    soft_reset();
                    break;
                case SDLK_x:
                    keys[0] = 1;
                    break;
                case SDLK_1:
                    keys[1] = 1;
                    break;
                case SDLK_2:
                    keys[2] = 1;
                    break;
                case SDLK_3:
                    keys[3] = 1;
                    break;
                case SDLK_q:
                    keys[4] = 1;
                    break;
                case SDLK_w:
                    keys[5] = 1;
                    break;
                case SDLK_e:
                    keys[6] = 1;
                    break;
                case SDLK_a:
                    keys[7] = 1;
                    break;
                case SDLK_s:
                    keys[8] = 1;
                    break;
                case SDLK_d:
                    keys[9] = 1;
                    break;
                case SDLK_z:
                    keys[10] = 1;
                    break;
                case SDLK_c:
                    keys[11] = 1;
                    break;
                case SDLK_4:
                    keys[12] = 1;
                    break;
                case SDLK_r:
                    keys[13] = 1;
                    break;
                case SDLK_f:
                    keys[14] = 1;
                    break;
                case SDLK_v:
                    keys[15] = 1;
                    break;
            }

        }
        // Process keyup events
        if (event.type == SDL_KEYUP) {
            switch(event.key.keysym.sym) {
                case SDLK_x:
                    keys[0] = 0;
                    break;
                case SDLK_1:
                    keys[1] = 0;
                    break;
                case SDLK_2:
                    keys[2] = 0;
                    break;
                case SDLK_3:
                    keys[3] = 0;
                    break;
                case SDLK_q:
                    keys[4] = 0;
                    break;
                case SDLK_w:
                    keys[5] = 0;
                    break;
                case SDLK_e:
                    keys[6] = 0;
                    break;
                case SDLK_a:
                    keys[7] = 0;
                    break;
                case SDLK_s:
                    keys[8] = 0;
                    break;
                case SDLK_d:
                    keys[9] = 0;
                    break;
                case SDLK_z:
                    keys[10] = 0;
                    break;
                case SDLK_c:
                    keys[11] = 0;
                    break;
                case SDLK_4:
                    keys[12] = 0;
                    break;
                case SDLK_r:
                    keys[13] = 0;
                    break;
                case SDLK_f:
                    keys[14] = 0;
                    break;
                case SDLK_v:
                    keys[15] = 0;
                    break;
            }
        }
    }

    return true;
}