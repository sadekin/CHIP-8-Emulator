#include "chip8.h"

Chip8::Chip8() {
    // Program counter starts at 0x200 because historically the system memory up to
    // 0x1FF was reserved for the interpreter itself. Most Chip-8 programs start
    // running at location 0x200.
    pc      = START_INSTRUCTION_ADDRESS;    // Reset program counter.
    opcode  = 0;                            // Reset current opcode.
    I       = 0;                            // Reset index register.
    sp      = 0;                            // Reset stack pointer.

    for (uint8_t&   i : display) i = 0;  // Clear display.
    for (uint8_t&   r : V)       r = 0;  // Clear registers.
    for (uint8_t&   m : memory)  m = 0;  // Clear memory.
    for (uint16_t&  m : stack)   m = 0;  // Clear stack.

    // Load font set.
    // The Chip-8 interpreter used a set of built-in fonts for
    // the hex digits 0 through F.
    // Each hexadecimal digit is represented using a 5x4 grid.
    // The 1s (bits set) represent where pixels would be on for that
    // character on a Chip-8 screen.
    uint8_t chip8_fontset[80] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    // These should be loaded into the memory at a predefined location,
    // usually starting at address 0x50 (or 0x000 in some references).
    for (int i = 0; i < 80; ++i) {
        memory[i + START_FONT_SET_ADDRESS] = chip8_fontset[i];
    }

    // Reset timers.
    delay_timer = 0;
    sound_timer = 0;
}

void Chip8::loadGame(const char *filename) {
    // Open the given file in binary mode and position the file pointer at the end.
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    // Check if the file was successfully opened.
    if (file.is_open()) {
        // Get the current position of the file pointer, which is the file size since
        // the file was opened with the file pointer at the end.
        std::streampos size = file.tellg();

        // Allocate a buffer in memory to hold the contents of the file.
        char* buffer = new char[size];

        // Reposition the file pointer to the beginning of the file.
        file.seekg(0, std::ios::beg);

        // Read the entire file into the buffer.
        file.read(buffer, size);

        // Close the file as its contents are now in the buffer, and it's no longer needed.
        file.close();

        // Copy the file contents from the buffer to the Chip-8 memory, starting at address 512 (0x200).
        for (int i = 0; i < size; ++i) {
            memory[i + 512] = buffer[i];
        }

        // Free the memory allocated for the buffer.
        delete[] buffer;
    }
}


void Chip8::emulateCycle() {
    // Fetch opcode
    opcode = memory[pc] << 8 | memory[pc + 1];

    // Decode opcode
    switch (opcode & 0xF000) {
        case 0x0000:
            // Handle 0x00E0 and 0x00EE
            break;
            //... Handle other opcodes

        default:
            std::cout << "Unknown opcode: " << opcode << std::endl;
    }

    // Update timers
    if (delay_timer > 0)
        --delay_timer;
    if (sound_timer > 0)
        --sound_timer;
}

uint8_t Chip8::mapSDLKeyToChip8Key(SDL_Keycode sdl_key) {
    switch (sdl_key) {
        // First row
        case SDLK_1: return 0x1;
        case SDLK_2: return 0x2;
        case SDLK_3: return 0x3;
        case SDLK_4: return 0xC;

        // Second row
        case SDLK_q: return 0x4;
        case SDLK_w: return 0x5;
        case SDLK_e: return 0x6;
        case SDLK_r: return 0xD;

        // Third row
        case SDLK_a: return 0x7;
        case SDLK_s: return 0x8;
        case SDLK_d: return 0x9;
        case SDLK_f: return 0xE;

        // Fourth row
        case SDLK_z: return 0xA;
        case SDLK_x: return 0x0;
        case SDLK_c: return 0xB;
        case SDLK_v: return 0xF;

        default: return 0xFF; // Indicates an invalid key
    }
}
