#include "chip8.h"

/* Do Nothing (Invalid Opcode) */
void Chip8::opcode_NONE() {
    printf("\nUnknown op code: %.4X\n", opcode);
    exit(3);
}


/* Display Opcodes */

// Clear the screen
void Chip8::opcode_00E0() {
    memset(display, 0, sizeof(display));
    drawFlag = true;
}

// Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels.
// Each row of 8 pixels is read as bit-coded starting from memory location I; I value does not
// change after the execution of this instruction.
// As described above, VF is set to 1 if any screen pixels are flipped from set to unset when
// the sprite is drawn, and to 0 if that does not happen.
void Chip8::opcode_DXYN() {
    uint8_t x = V[(opcode & 0x0F00) >> 8];
    uint8_t y = V[(opcode & 0x00F0) >> 4];
    uint8_t height = opcode & 0x000F;

    // Reset VF - will be set to 1 if any collision occurs during drawing
    V[0xF] = 0;

    for (uint8_t row = 0; row < height; ++row) {
        uint8_t spriteByte = memory[I + row];

        // Loop through each bit (pixel) in the byte
        for (uint8_t col = 0; col < 8; ++col) {
            bool spritePixelIsOn = (spriteByte & (0x80 >> col)) != 0;
            uint8_t* screenPixel = &display[x + col + (y + row) * DISPLAY_WIDTH];

            // Detect collision
            if (spritePixelIsOn) {
                if (*screenPixel) {
                    V[0xF] = 1;
                }
                *screenPixel ^= 1;
            }
        }
    }
    drawFlag = true;
}

/* Flow Opcodes */

// Return from a subroutine
void Chip8::opcode_00EE() {
    // When we return from a subroutine, we need to pop the top address from the stack
    // and jump back to it, hence pre-decrementing the stack pointer
    pc = stack[--sp];
}

// Jump to address NNN
void Chip8::opcode_1NNN() {
    uint16_t address = opcode & 0x0FFF;
    pc = address;
}

// Call subroutine at NNN
void Chip8::opcode_2NNN() {
    stack[sp++] = pc;
    uint16_t address = opcode & 0x0FFF;
    pc = address;
}

// Jump to address NNN plus V0
void Chip8::opcode_BNNN() { pc = (opcode & 0x0FFF) + V[0]; }


/* Conditional Opcodes */

// Skip next instruction if VX equals NN
void Chip8::opcode_3XNN() {
    if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) pc += 2;
}

// Skip next instruction if VX doesn't equal NN
void Chip8::opcode_4XNN() {
    if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) pc += 2;
}

// Skip next instruction if VX equals VY
void Chip8::opcode_5XY0() {
    if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4]) pc += 2;
}

// Skip next instruction if VX doesn't equal VY
void Chip8::opcode_9XY0() {
    if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4]) pc += 2;
}

// Skip next instruction if key with value VX is pressed
void Chip8::opcode_EX9E() {
    if (key[V[(opcode & 0x0F00) >> 8]]) pc += 2;
}

// Skip next instruction if key with value VX is not pressed
void Chip8::opcode_EXA1() {
    if (!key[V[(opcode & 0x0F00) >> 8]]) pc += 2;
}


/* Arithmetic Opcodes */

// Set VX to NN
void Chip8::opcode_6XNN() { V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF; }

// Adds NN to VX (carry flag is not changed).
void Chip8::opcode_7XNN() { V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF; }

// Sets VX to the value of VY.
void Chip8::opcode_8XY0() { V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4]; }

// Sets VX to VX or VY. (bitwise OR operation)
void Chip8::opcode_8XY1() { V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4]; }

// Sets VX to VX and VY. (bitwise AND operation)
void Chip8::opcode_8XY2() { V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4]; }

// Set VX to VX xor VY
void Chip8::opcode_8XY3() { V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4]; }

// Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there is not.
void Chip8::opcode_8XY4() {
    uint8_t rx = (opcode & 0x0F00) >> 8;
    uint8_t ry = (opcode & 0x00F0) >> 4;

    uint16_t sum = V[rx] + V[ry];
    V[0xF] = sum > 0xFF ? 1 : 0;

    V[rx] = sum & 0xFF;
}

// VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there is not.
void Chip8::opcode_8XY5() {
    uint8_t rx = (opcode & 0x0F00) >> 8;
    uint8_t ry = (opcode & 0x00F0) >> 4;

    V[0xF] = V[rx] > V[ry] ? 1 : 0;

    V[rx] -= V[ry];
}

// Stores the least significant bit of VX in VF and then shifts VX to the right by 1.
void Chip8::opcode_8XY6() {
    uint8_t rx = (opcode & 0x0F00) >> 8;
    V[0xF] = V[rx] & 0x1;
    V[rx] >>= 1;
}

// Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there is not.
void Chip8::opcode_8XY7() {
    uint8_t rx = (opcode & 0x0F00) >> 8;
    uint8_t ry = (opcode & 0x00F0) >> 4;

    V[0xF] = V[ry] > V[rx] ? 1 : 0;

    V[rx] = V[ry] - V[rx];
}

// Stores the most significant bit of VX in VF and then shifts VX to the left by 1.
void Chip8::opcode_8XYE() {
    uint8_t rx = (opcode & 0x0F00) >> 8;
    V[0xF] = (V[rx] & 0x80) >> 7;
    V[rx] <<= 1;
}


/* Random Opcode */

// Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN.
void Chip8::opcode_CXNN() {
    V[(opcode & 0x0F00) >> 8] = randByte(randEngine) & (opcode & 0x00FF);
}


/* Timer Opcodes */

// Set VX to the value of the delay timer.
void Chip8::opcode_FX07() { V[(opcode & 0x0F00) >> 8] = delayTimer; }

// A key press is awaited, and then stored in VX (blocking operation, all instruction halted until next key event).
void Chip8::opcode_FX0A() {
    bool keyPressed = false;
    for (uint8_t i = 0; i < NUM_KEYS; i++) {
        if (key[i]) {
            V[(opcode & 0x0F00) >> 8] = i;
            keyPressed = true;
            break;
        }
    }
    if (!keyPressed) pc -= 2; // No key was pressed; all instruction halted until next key event.
}

// Sets the delay timer to VX.
void Chip8::opcode_FX15() { delayTimer = V[(opcode & 0x0F00) >> 8]; }

// Sets the sound timer to VX.
void Chip8::opcode_FX18() { soundTimer = V[(opcode & 0x0F00) >> 8]; }

// Adds VX to I. VF is not affected.
void Chip8::opcode_FX1E() { I += V[(opcode & 0x0F00) >> 8]; }


/* Memory Opcodes */

// Sets I to the address NNN.
void Chip8::opcode_ANNN() { I = opcode & 0x0FFF; }

// Sets I to the location of the sprite for the character in VX.
// Characters 0-F (in hexadecimal) are represented by a 4x5 font.
void Chip8::opcode_FX29() {
    uint8_t rx = (opcode & 0x0F00) >> 8;

    // Font chars are located at START_FONT_SET_ADDRESS (offset) and are 5 bytes each
    uint16_t spriteAddr = START_FONT_SET_ADDRESS + (V[rx] * 5);
    I = spriteAddr;
}

// Stores the binary-coded decimal (BCD) representation of VX,
// with the hundreds digit in memory at location in I,
// the tens digit at location I+1, and the ones digit at location I+2.
void Chip8::opcode_FX33() {
    uint8_t value   = V[(opcode & 0x0F00) >> 8];
    memory[I]       = value / 100;
    memory[I + 1]   = (value % 100) / 10;
    memory[I + 2]   = value % 10;
}

// Stores from V0 to VX (including VX) in memory, starting at address I.
// The offset from I is increased by 1 for each value written, but I itself is left unmodified.
void Chip8::opcode_FX55() {
    uint8_t rx = (opcode & 0x0F00) >> 8;
    for (int i = 0; i < rx + 1; ++i) memory[I + i] = V[i];
}

// Fills from V0 to VX (including VX) with values from memory, starting at address I.
// The offset from I is increased by 1 for each value read, but I itself is left unmodified.
void Chip8::opcode_FX65() {
    uint8_t rx = (opcode & 0x0F00) >> 8;
    for (int i = 0; i < rx + 1; ++i) V[i] = memory[I + i];
}


/* Opcode Table Initialization */

void Chip8::tabulateOpcodes() {
    // The first digit of each opcode runs from 0x0 t0 0xF, hence sizeof(table) = 0xF + 1;
    table[0x0] = &Chip8::Table0;        // See (*) below

    table[0x1] = &Chip8::opcode_1NNN;
    table[0x2] = &Chip8::opcode_2NNN;
    table[0x3] = &Chip8::opcode_3XNN;
    table[0x4] = &Chip8::opcode_4XNN;
    table[0x5] = &Chip8::opcode_5XY0;
    table[0x6] = &Chip8::opcode_6XNN;
    table[0x7] = &Chip8::opcode_7XNN;

    table[0x8] = &Chip8::Table8;        // See (*) below

    table[0x9] = &Chip8::opcode_9XY0;
    table[0xA] = &Chip8::opcode_ANNN;
    table[0xB] = &Chip8::opcode_BNNN;
    table[0xC] = &Chip8::opcode_CXNN;
    table[0xD] = &Chip8::opcode_DXYN;

    table[0xE] = &Chip8::TableE;        // See (*) below
    table[0xF] = &Chip8::TableF;        // See (*) below

    // (*) For the opcodes with first digits that repeat ($0, $8, $E, $F),
    // we’ll need secondary tables that can accommodate each of those.
    // The opcodes that are unused are filled with opcode_NONE do indicate an invalid opcode (doing nothing).
    for (size_t i = 0; i < 0xE + 1; i++) {
        table0[i] = table8[i] = tableE[i] = &Chip8::opcode_NONE;
    }

    // $0 needs an array that can index up to $E+1
    table0[0x0] = &Chip8::opcode_00E0;
    table0[0xE] = &Chip8::opcode_00EE;

    // $8 needs an array that can index up to $E+1
    table8[0x0] = &Chip8::opcode_8XY0;
    table8[0x1] = &Chip8::opcode_8XY1;
    table8[0x2] = &Chip8::opcode_8XY2;
    table8[0x3] = &Chip8::opcode_8XY3;
    table8[0x4] = &Chip8::opcode_8XY4;
    table8[0x5] = &Chip8::opcode_8XY5;
    table8[0x6] = &Chip8::opcode_8XY6;
    table8[0x7] = &Chip8::opcode_8XY7;
    table8[0xE] = &Chip8::opcode_8XYE;

    // $E needs an array that can index up to $E+1
    tableE[0x1] = &Chip8::opcode_EXA1;
    tableE[0xE] = &Chip8::opcode_EX9E;

    // $F needs an array that can index up to $65+1
    for (Opcode& f : tableF) f = &Chip8::opcode_NONE;
    tableF[0x07] = &Chip8::opcode_FX07;
    tableF[0x0A] = &Chip8::opcode_FX0A;
    tableF[0x15] = &Chip8::opcode_FX15;
    tableF[0x18] = &Chip8::opcode_FX18;
    tableF[0x1E] = &Chip8::opcode_FX1E;
    tableF[0x29] = &Chip8::opcode_FX29;
    tableF[0x33] = &Chip8::opcode_FX33;
    tableF[0x55] = &Chip8::opcode_FX55;
    tableF[0x65] = &Chip8::opcode_FX65;
}


/* Opcode Retrieval */

void Chip8::Table0() { (this->*table0[opcode & 0x000F])(); }

void Chip8::Table8() { (this->*table8[opcode & 0x000F])(); }

void Chip8::TableE() { (this->*tableE[opcode & 0x000F])(); }

void Chip8::TableF() { (this->*tableF[opcode & 0x00FF])(); }

