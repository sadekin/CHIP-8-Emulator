#include "chip8.h"

int main() {
    Chip8 chip8;

    // Load ROM into memory
    chip8.LoadROM("test_opcode.ch8");

    sf::RenderWindow window(sf::VideoMode(DISPLAY_WIDTH * 10, DISPLAY_HEIGHT * 10), "Chip8 Emulator");

    while (window.isOpen()) {
        chip8.HandleInput(window);
        chip8.Cycle();

        if (chip8.drawFlag) {
            chip8.Render(window);
        }

        // Add a delay or limit the frame rate so it doesn't run too fast
        sf::sleep(sf::milliseconds(1));
    }

    return 0;
}




