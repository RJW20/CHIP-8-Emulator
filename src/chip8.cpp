#include <iostream>
#include <cstdint>
#include <string>
#include <fstream>
#include <chrono>
#include <thread>
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

/* Initialise SDL, load the ROM into memory and set all components to their
initial values. */
Chip8::Chip8(std::string file_path) {
    initialise_SDL();
    rom_path = file_path;
    hard_reset();
}

// Destroy all SDL components
Chip8::~Chip8() {
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

// Initialise all SDL components
void Chip8::initialise_SDL() {

    // Initialise SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError()
            << std::endl;
        exit(1);
    };

    // Create SDL window
    window = SDL_CreateWindow(
        "CHIP-8 Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, 0
    );
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " <<
            SDL_GetError() << std::endl;
        exit(2);
    }

    // Create SDL renderer
    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " <<
            SDL_GetError() << std::endl;
        exit(3);
    }

    // Create a texture to hold the pixel data
    texture = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING,
        64, 32
    );
    if (!texture) {
        std::cerr << "Texture could not be created! SDL_Error: " <<
            SDL_GetError() << std::endl;
        exit(4);
    }
}

// Load the ROM into memory and soft reset
void Chip8::hard_reset() {

    std::cout << "Loading ROM: " << rom_path << std::endl;

    // Open the file in binary mode
    std::ifstream file(rom_path, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Failed to open ROM: " << rom_path << std::endl;
        exit(5);
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
        exit(6);
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

    for (int i = 0; i < 32; ++i) {
        for (int j = 0; j < 64; ++j) {
            display[i][j] = 0;
        }
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

    using namespace std;
    using namespace std::chrono;

    using time_between_frames = duration<int64_t, ratio<1, 60>>;    // 60 Hz
    auto next_frame = system_clock::now() + time_between_frames{0};
    auto last_frame = next_frame - time_between_frames{1};
    
    while(true) {

        for (int i = 0; i < APF; ++i) {

            if (!handle_events()) {
                return;
            }
            advance();
        }

        render();

        if (delay > 0) {
            delay -=1;
        }
        if (sound > 0) {
            sound -= 1;
        }

        this_thread::sleep_until(next_frame);
        last_frame = next_frame;
        next_frame = next_frame + time_between_frames{1};
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

// Emulate one fetch/decode/execute loop
void Chip8::advance() {

    uint16_t opcode = memory[pc] << 8 | memory[pc + 1];
    pc += 2;

    uint16_t X = (opcode & 0xF00) >> 8;
    uint16_t Y = (opcode & 0xF0) >> 4;
    uint16_t N = opcode & 0xF;
    uint16_t NN = opcode & 0xFF;
    uint16_t NNN = opcode & 0xFFF;
    switch(opcode & 0xF000) {

        // 00EN
        case 0x0000:

            switch(N) {
                
                // 00E0 - clear screen
                case 0x0000:

                    for (int i = 0; i < 32; ++i) {
                        for (int j = 0; j < 64; ++j) {
                            display[i][j] = 0;
                        }
                    }
                    break;

            }
            break;

        // 1NNN - jump to NNN
        case 0x1000:
            pc = NNN;
            break;

        case 0x2000:
            break;

        case 0x3000:
            break;

        case 0x4000:
            break;

        case 0x5000:
            break;

        // 6XNN - set VX to NN
        case 0x6000:
            V[X] = NN;
            break;

        // 7XNN - add NN to VX
        case 0x7000:
            V[X] += NN;
            break;

        case 0x8000:
            break;

        case 0x9000:
            break;

        // ANNN - set I to NNN
        case 0xA000:
            I = NNN;
            break;

        case 0xB000:
            break;

        case 0xC000:
            break;

        // DXYN - draw sprite at the memory address in I that is (8, N) pixels
        // wide/tall to (VX, VY).
        case 0xD000: {

            uint8_t x = V[X] & 0x3F;
            uint8_t y = V[Y] & 0x1F;
            
            V[0xF] = 0;
            uint8_t pixels;
            for (int row = 0; row < N; ++row) {

                if (y + row > 31) {
                    break;
                }

                pixels = memory[I + row];
                for (int col = 0; col < 8; ++col) {

                    if (x + col > 63) {
                        break;
                    }

                    if ((pixels >> (7 - col)) & 1 == 1) {

                        if (display[y + row][x + col]) {
                            V[0xF] = 1;
                        }

                        display[y + row][x + col] ^= 1;

                    }
                }
            }
        }
            break;

        case 0xE000:
            break;

        case 0xF000:
            break;

    }

}

// Draw the display to the window
void Chip8::render() {

    // Lock the texture to get direct access to its pixels
    void* pixel_pointer;
    int pitch;
    if (SDL_LockTexture(texture, nullptr, &pixel_pointer, &pitch) != 0) {
        std::cerr << "SDL_LockTexture failed: " << SDL_GetError() << std::endl;
    }

    // Modify the pixel data
    uint32_t* pixel_buffer = static_cast<uint32_t*>(pixel_pointer);
    for (int i = 0; i < 32; ++i) {
        for (int j = 0; j < 64; ++j) {
            pixel_buffer[i * 64 + j] = (0x00FFFFFF * display[i][j]) | 0xFF000000;
        }
    }

    SDL_UnlockTexture(texture);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
}