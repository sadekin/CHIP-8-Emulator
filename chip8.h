#pragma once

#include <iostream>
#include <fstream>
#include <SDL.h>
#include <SDL_ttf.h>
#include <random>
#include <chrono>

const unsigned int RAM_SIZE         = 4096;
const unsigned int NUM_REGISTERS    = 16;
const unsigned int DISPLAY_WIDTH    = 64;
const unsigned int DISPLAY_HEIGHT   = 32;
const unsigned int STACK_LEVELS     = 16;
const unsigned int NUM_KEYS         = 16;

const unsigned int START_INSTRUCTION_ADDRESS    = 0x200;
const unsigned int START_FONT_SET_ADDRESS       = 0x50;
const unsigned int FONT_SET_SIZE                = 80;


class Chip8 {
public:
    Chip8();
    void loadROM(const char* filename);
    void Cycle();

    uint8_t display[DISPLAY_WIDTH * DISPLAY_HEIGHT]{};  // Monochrome display of 64x32 pixels (2048 pixels total)
    uint8_t key[NUM_KEYS]{};                            // Represents state of 16 keys; 0/1 = unpressed/pressed

private:
    uint16_t opcode{};                                  // Current opcode
    uint8_t memory[RAM_SIZE]{};                         // 4K memory of the Chip-8 system
    uint8_t V[NUM_REGISTERS]{};                         // 16 general-purpose 8-bit registers. VF doubles as a flag.
    uint16_t I{};                                       // Index register
    uint16_t pc;                                        // Program counter
    uint8_t delay_timer{};                              // Delay timer, decrements at 60Hz when set to a value above 0
    uint8_t sound_timer{};                              // Sound timer, system beeps when this timer reaches 0
    uint16_t stack[STACK_LEVELS]{};                     // Stack for storing return addresses
    uint16_t sp{};                                      // Stack pointer

    std::default_random_engine randGen;                 // See opcode_CXNN
    std::uniform_int_distribution<uint8_t> randByte;    // See opcode_CXNN

    void Table0();
    void Table8();
    void TableE();
    void TableF();
    typedef void (Chip8::*Opcode)();
    Opcode table[0xF + 1]{};
    Opcode table0[0xE + 1]{};
    Opcode table8[0xE + 1]{};
    Opcode tableE[0xE + 1]{};
    Opcode tableF[0x65 + 1]{};
    void tabulateOpcodes();


    static uint8_t mapSDLKeyToChip8Key(SDL_Keycode sdl_key);

    // Opcodes===============================================================================================
    // https://en.wikipedia.org/wiki/CHIP-8#Opcode_table

    /* Do Nothing (Invalid Opcode) */
    void opcode_NONE();

    /* Display Opcodes */
    void opcode_00E0();     // Clear the screen
    void opcode_DXYN();     // Draw a sprite

    /* Flow Opcodes */
    void opcode_00EE();     // Return from a subroutine
    void opcode_1NNN();     // Jump to address NNN
    void opcode_2NNN();     // Call subroutine at NNN
    void opcode_BNNN();     // Jump to address NNN plus V0

    /* Conditional Opcodes */
    void opcode_3XNN();     // Skip next instruction if VX equals NN
    void opcode_4XNN();     // Skip next instruction if VX doesn't equal NN
    void opcode_5XY0();     // Skip next instruction if VX equals VY
    void opcode_9XY0();     // Skip next instruction if VX doesn't equal VY
    void opcode_EX9E();     // Skip next instruction if key with value VX is pressed
    void opcode_EXA1();     // Skip next instruction if key with value VX is not pressed

    /* Arithmetic Opcodes */
    void opcode_6XNN();     // Set VX to NN
    void opcode_7XNN();     // Add NN to VX
    void opcode_8XY0();     // Set VX to VY
    void opcode_8XY1();     // Set VX to VX or VY
    void opcode_8XY2();     // Set VX to VX and VY
    void opcode_8XY3();     // Set VX to VX xor VY
    void opcode_8XY4();     // Add VY to VX with carry flag
    void opcode_8XY5();     // Subtract VY from VX with borrow flag
    void opcode_8XY6();     // Shift VX right
    void opcode_8XY7();     // Set VX to VY minus VX
    void opcode_8XYE();     // Shift VX left

    /* Random Opcodes */
    void opcode_CXNN();     // Set VX to a random number bitwise and NN

    /* Timer Opcodes */
    void opcode_FX07();     // Set VX to the value of the delay timer
    void opcode_FX0A();     // Wait for a key press and store in VX
    void opcode_FX15();     // Set delay timer to VX
    void opcode_FX18();     // Set sound timer to VX
    void opcode_FX1E();     // Add VX to I

    /* Memory Opcodes */
    void opcode_ANNN();     // Set I to address NNN
    void opcode_FX29();     // Set I to the location of the sprite for character in VX
    void opcode_FX33();     // Store binary-coded decimal representation of VX
    void opcode_FX55();     // Store registers V0 through VX in memory
    void opcode_FX65();     // Read registers V0 through VX from memory
};