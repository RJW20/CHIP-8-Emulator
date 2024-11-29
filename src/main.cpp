#include <iostream>
#include <string>
#include "chip8.hpp"

int main(int argc, char** argv) {

    // Check for rom provided
    if (argc != 2) {
        std::cout << "Usage: chip8 <ROM file>" << std::endl;
        return 1;
    }

    Chip8 chip8((std::string) argv[1]);
    chip8.run();

    return 0;
}