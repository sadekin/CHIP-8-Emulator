#pragma once

#include <iostream>
#include <fstream>
#include <random>   // for opcode Cxkk
#include <chrono>   // for random seed

const unsigned int RAM_SIZE         = 4096;
const unsigned int REGISTER_COUNT   = 16;
const unsigned int DISPLAY_WIDTH    = 64;
const unsigned int DISPLAY_HEIGHT   = 32;
const unsigned int STACK_LEVELS     = 16;
const unsigned int KEY_COUNT        = 16;

const unsigned int START_INSTRUCTION_ADDRESS    = 0x200;
const unsigned int START_FONT_SET_ADDRESS       = 0x50;
const unsigned int FONT_SET_SIZE                = 80;


class Chip8 {
public:
    Chip8();

    void LoadROM(const std::string& filename);
    void Cycle();
    void Reset();

    uint8_t display[DISPLAY_WIDTH * DISPLAY_HEIGHT]{};  // Monochrome display of 64x32 pixels (2048 pixels total)
    uint8_t key[KEY_COUNT]{};                           // Represents state of 16 keys; 0/1 = unpressed/pressed

    bool drawFlag{};                                    // Signal to draw

private:
    uint8_t memory[RAM_SIZE]{};                         // 4K memory of the Chip-8 system
    uint8_t V[REGISTER_COUNT]{};                        // 16 general-purpose 8-bit registers. VF doubles as a flag.

    uint16_t pc;                                        // Program counter
    uint16_t opcode{};                                  // Current opcode
    uint16_t I{};                                       // Index register

    uint16_t stack[STACK_LEVELS]{};                     // Stack for storing return addresses
    uint16_t sp{};                                      // Stack pointer

    uint8_t delayTimer{};                               // Delay timer, decrements at 60Hz when set to a value above 0
    uint8_t soundTimer{};                               // Sound timer, system beeps when this timer reaches 0

    std::default_random_engine randEngine;              // RNG (see opcode_Cxkk)
    std::uniform_int_distribution<uint8_t> randByte;    // Random byte generator (see opcode_Cxkk)

    // I tabularize the opcodes in accordance with the technique discussed by
    // Austin Morlan in his CHIP-8 tutorial (see README). Each table consists
    // of function pointers to the opcode methods. The first table also contains
    // function pointers to Table0, Table8, TableE and TableF for further decoding
    // of an opcode via a bitmask if needed.

    typedef void (Chip8::*Opcode)();
    Opcode table[0xF + 1]{};
    Opcode table0[0xE + 1]{};
    Opcode table8[0xE + 1]{};
    Opcode tableE[0xE + 1]{};
    Opcode tableF[0x65 + 1]{};
    void Table0();
    void Table8();
    void TableE();
    void TableF();
    void tabulateOpcodes();

    // Opcodes===========================================================================
    // "The original implementation of the Chip-8 language includes 36 different
    // instructions, including math, graphics, and flow control functions.
    // All instructions are 2 bytes long and are stored most-significant-byte first.
    // In memory, the first byte of each instruction should be located at an even addresses.
    // If a program includes sprite data, it should be padded so any instructions following
    // it will be properly situated in RAM." - Cowgod
    //
    // Opcodes have been listed in the order that they are seen in Cowgod's Technical
    // Reference (see README) for ease of finding. opcode_NONE was included to cover the
    // possibility of an invalid/unknown instruction. I have included the description of
    // each instruction above each implementation. Verbosity in comments stems from my
    // own ignorance of emulation and a desire to deepen my understanding of it.

    void opcode_00E0();     void opcode_8xyE();     void opcode_NONE();
    void opcode_00EE();     void opcode_9xy0();
    void opcode_1nnn();     void opcode_Annn();
    void opcode_2nnn();     void opcode_Bnnn();
    void opcode_3xkk();     void opcode_Cxkk();
    void opcode_4xkk();     void opcode_Dxyn();
    void opcode_5xy0();     void opcode_Ex9E();
    void opcode_6xkk();     void opcode_ExA1();
    void opcode_7xkk();     void opcode_Fx07();
    void opcode_8xy0();     void opcode_Fx0A();
    void opcode_8yx1();     void opcode_Fx15();
    void opcode_8xy2();     void opcode_Fx18();
    void opcode_8xy3();     void opcode_Fx1E();
    void opcode_8xy4();     void opcode_Fx29();
    void opcode_8xy5();     void opcode_Fx33();
    void opcode_8xy6();     void opcode_Fx65();
    void opcode_8xy7();     void opcode_Fx55();

    // Opcode Utility Functions=========================================================
    // These are helper functions designed to aid with the extraction of information
    // from an opcode.
    //
    // nnn or addr  - A 12-bit value, the lowest 12 bits of the instruction
    // n or nibble  - A 4-bit value, the lowest 4 bits of the instruction
    // x            - A 4-bit value, the lower 4 bits of the high byte of the instruction
    // y            - A 4-bit value, the upper 4 bits of the low byte of the instruction
    // kk or byte   - An 8-bit value, the lowest 8 bits of the instruction

    [[nodiscard]] uint16_t getNNN() const { return opcode & 0x0FFF; }       // Address
    [[nodiscard]] uint8_t getX() const { return (opcode & 0x0F00) >> 8;}    // Vx
    [[nodiscard]] uint8_t getY() const { return (opcode & 0x00F0) >> 4;}    // Vy
    [[nodiscard]] uint8_t getKK() const { return opcode & 0x00FF; }         // Byte
};