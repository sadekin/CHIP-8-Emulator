#include "chip8.h"

// The Chip-8 interpreter used a set of built-in fonts for
// the hex digits 0 through F.
// Each hexadecimal digit is represented using a 5x4 grid.
// The 1s (bits set) represent where pixels would be on for that
// character on a Chip-8 screen.
static const uint8_t chip8_font_set[FONT_SET_SIZE] = {
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



Chip8::Chip8()
    : randEngine(std::chrono::system_clock::now().time_since_epoch().count())  {
    // Program counter starts at 0x200 because historically the system memory up to
    // 0x1FF was reserved for the interpreter itself. Most Chip-8 programs start
    // running at location 0x200.
    pc = START_INSTRUCTION_ADDRESS;

    // Font set should be loaded into the memory at a predefined location,
    // usually starting at address 0x50 (or 0x000 in some references).
    for (int i = 0; i < FONT_SET_SIZE; ++i) {
        memory[i + START_FONT_SET_ADDRESS] = chip8_font_set[i];
    }

    // Initialize random byte generator for opcode_Cxkk
    randByte = std::uniform_int_distribution<uint8_t>(0, 255);

    tabulateOpcodes();
}

void Chip8::Reset() {
    pc = START_INSTRUCTION_ADDRESS;
    opcode = I = sp = delayTimer = soundTimer = 0;

    memset(memory, 0, RAM_SIZE);
    memset(V, 0, REGISTER_COUNT);
    memset(stack, 0, STACK_LEVELS);
    memset(key, 0, KEY_COUNT);
    memset(display, 0, sizeof(display));

    // Font set should be loaded into the memory at a predefined location,
    // usually starting at address 0x50 (or 0x000 in some references).
    for (int i = 0; i < FONT_SET_SIZE; ++i) {
        memory[i + START_FONT_SET_ADDRESS] = chip8_font_set[i];
    }
}


void Chip8::LoadROM(const char *filename) {
    Reset();
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
            memory[i + START_INSTRUCTION_ADDRESS] = buffer[i];
        }

        // Free the memory allocated for the buffer.
        delete[] buffer;
    }
}


void Chip8::Cycle() {
    // Fetch opcode
    opcode = memory[pc] << 8 | memory[pc + 1]; // big endian

    // Increment PC before execution
    pc += 2;

    // Decode opcode
    (this->*table[(opcode & 0xF000) >> 12])();

    // Update timers
    if (delayTimer > 0) --delayTimer;
    if (soundTimer > 0) --soundTimer;
}