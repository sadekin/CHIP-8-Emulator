#include "chip8.h"

/* Opcodes */

// 00E0 - CLS: Clear the display.
void Chip8::opcode_00E0() {
    memset(display, 0, sizeof(display));
    drawFlag = true;
}

// 00EE - RET: Return from a subroutine.
void Chip8::opcode_00EE() { pc = stack[--sp]; }

// 1nnn - JP addr: Jump to location nnn. The interpreter sets the program counter to nnn.
void Chip8::opcode_1nnn() { pc = getNNN(); }

// 2nnn - CALL addr: Call subroutine at nnn. The interpreter increments the SP,
// then puts the current PC on the top of the stack. The PC is then set to nnn.
void Chip8::opcode_2nnn() {
    stack[sp++] = pc;
    pc = getNNN();
}

// 3xkk - SE Vx, byte: Skip next instruction if Vx = kk.
// The interpreter compares register Vx to kk, and if they are equal, increments the program counter by 2.
void Chip8::opcode_3xkk() { if (V[getX()] == getKK()) pc += 2; }

// 4xkk - SNE Vx, byte: Skip next instruction if Vx != kk.
void Chip8::opcode_4xkk() { if (V[getX()] != getKK()) pc += 2; }

// 5xy0 - SE Vx, Vy: Skip next instruction if Vx = Vy.
// The interpreter compares register Vx to register Vy, and if they are equal, increments the program counter by 2.
void Chip8::opcode_5xy0() { if (V[getX()] == V[getY()]) pc += 2; }

// 6xkk - LD Vx, byte: The interpreter puts the value kk into register Vx.
void Chip8::opcode_6xkk() { V[getX()] = getKK(); }

// 7xkk - ADD Vx, byte: Set Vx = Vx + kk. (Carry flag is not affected).
void Chip8::opcode_7xkk() { V[getX()] += getKK(); }

// 8xy0 - LD Vx, Vy: Set Vx = Vy. Stores the value of register Vy in register Vx.
void Chip8::opcode_8xy0() { V[getX()] = V[getY()]; }

// 8xy1 - OR Vx, Vy: Set Vx = Vx OR Vy. (Bitwise OR)
void Chip8::opcode_8yx1() { V[getX()] |= V[getY()]; }

// 8xy2 - AND Vx, Vy: Set Vx = Vx AND Vy. (Bitwise AND)
void Chip8::opcode_8xy2() { V[getX()] &= V[getY()]; }

// 8xy3 - XOR Vx, Vy: Set Vx = Vx XOR Vy.
void Chip8::opcode_8xy3() { V[getX()] ^= V[getY()]; }

// 8xy4 - ADD Vx, Vy: Set Vx = Vx + Vy, set VF = carry.
// The values of Vx and Vy are added together. If the result is greater than 8 bits (i.e., > 255)
// VF is set to 1, otherwise 0. Only the lowest 8 bits of the result are kept, and stored in Vx.
void Chip8::opcode_8xy4() {
    uint8_t rx = getX(), ry = getY();

    uint16_t sum = V[rx] + V[ry];
    V[0xF] = sum > 0xFF ? 1 : 0;

    V[rx] = sum & 0xFF;
}

// 8xy5 - SUB Vx, Vy: Set Vx = Vx - Vy, set VF = NOT borrow.
// If Vx > Vy, then VF is set to 1, otherwise 0. Then Vy is subtracted from Vx, and the result is stored in Vx.
void Chip8::opcode_8xy5() {
    uint8_t rx = getX(), ry = getY();

    V[0xF] = V[rx] > V[ry] ? 1 : 0;

    V[rx] -= V[ry];
}

// 8xy6 - SHR Vx {, Vy}: Set Vx = Vx SHR 1.
// If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0. Then Vx is divided by 2.
void Chip8::opcode_8xy6() {
    uint8_t rx = getX();
    V[0xF] = V[rx] & 0x1;
    V[rx] >>= 1;
}

// 8xy7 - SUBN Vx, Vy: Set Vx = Vy - Vx, set VF = NOT borrow.
// If Vy > Vx, then VF is set to 1, otherwise 0. Then Vx is subtracted from Vy, and the result is stored in Vx.
void Chip8::opcode_8xy7() {
    uint8_t rx = getX(), ry = getY();

    V[0xF] = V[ry] > V[rx] ? 1 : 0;

    V[rx] = V[ry] - V[rx];
}

// 8xyE - SHL Vx {, Vy}: Set Vx = Vx SHL 1.
// If the most-significant bit of Vx is 1, then VF is set to 1, otherwise to 0. Then Vx is multiplied by 2.
void Chip8::opcode_8xyE() {
    uint8_t rx = getX();
    V[0xF] = (V[rx] & 0x80) >> 7;
    V[rx] <<= 1;
}

// 9xy0 - SNE Vx, Vy: Skip next instruction if Vx != Vy.
void Chip8::opcode_9xy0() { if (V[getX()] != V[getY()]) pc += 2; }

// Annn - LD I, addr: Set I = nnn.
void Chip8::opcode_Annn() { I = getNNN(); }

// Bnnn - JP V0, addr: Jump to location nnn + V0.
void Chip8::opcode_Bnnn() { pc = getNNN() + V[0]; }

// Cxkk - RND Vx, byte: Set Vx = random byte AND kk.
void Chip8::opcode_Cxkk() { V[getX()] = randByte(randEngine) & getKK(); }

// Dxyn - DRW Vx, Vy, nibble: Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
// Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels.
// Each row of 8 pixels is read as bit-coded starting from memory location I; I value does not
// change after the execution of this instruction.
// VF is set to 1 if any screen pixels are flipped from set to unset when
// the sprite is drawn, and to 0 if that does not happen.
void Chip8::opcode_Dxyn() {
    uint8_t x = V[getX()], y = V[getY()];
    uint8_t height = opcode & 0x000F;

    V[0xF] = 0; // reset VF in case collision does not occur

    for (uint8_t row = 0; row < height; ++row) {
        uint8_t spriteByte = memory[I + row];

        // Loop through each bit (pixel) in the byte
        for (uint8_t col = 0; col < 8; ++col) {
            bool spritePixelIsOn = (spriteByte & (0x80 >> col)) != 0;
            uint8_t* screenPixel = &display[x + col + (y + row) * DISPLAY_WIDTH];

            if (spritePixelIsOn) {
                if (*screenPixel) V[0xF] = 1; // collision
                *screenPixel ^= 1;
            }
        }
    }
    drawFlag = true;
}

// Ex9E - SKP Vx: Skip next instruction if key with the value of Vx is pressed.
void Chip8::opcode_Ex9E() { if (key[V[getX()]]) pc += 2; }

// ExA1 - SKNP Vx: Skip next instruction if key with the value of Vx is not pressed.
void Chip8::opcode_ExA1() { if (!key[V[getX()]]) pc += 2; }

// Fx07 - LD Vx, DT: Set Vx = delay timer value.
void Chip8::opcode_Fx07() { V[getX()] = delayTimer; }

// Fx0A - LD Vx, K: Wait for a key press, store the value of the key in Vx.
// All execution stops until a key is pressed, then the value of that key is stored in Vx.
void Chip8::opcode_Fx0A() {
    for (uint8_t i = 0; i < KEY_COUNT; i++) {
        if (key[i]) {
            V[getX()] = i;
            return;
        }
    }
    pc -= 2; // no key was pressed
}

// Fx15 - LD DT, Vx: Set delay timer = Vx.
void Chip8::opcode_Fx15() { delayTimer = V[getX()]; }

// Fx15 - LD DT, Vx: Set delay timer = Vx.
void Chip8::opcode_Fx18() { soundTimer = V[getX()]; }

// Fx1E - ADD I, Vx: Set I = I + Vx.
void Chip8::opcode_Fx1E() { I += V[getX()]; }

// Fx29 - LD F, Vx: Set I = location of sprite for digit Vx.
// Characters 0-F (in hexadecimal) are represented by a 4x5 font.
void Chip8::opcode_Fx29() {
    uint8_t digit = V[getX()];

    // Font chars are located at START_FONT_SET_ADDRESS (offset) and are 5 bytes each
    uint16_t spriteAddr = START_FONT_SET_ADDRESS + (digit * 5);
    I = spriteAddr;
}

// Fx33 - LD B, Vx: Store BCD representation of Vx in memory locations I, I+1, and I+2.
void Chip8::opcode_Fx33() {
    uint8_t value   = V[getX()];
    memory[I]       = value / 100;          // hundreds
    memory[I + 1]   = (value % 100) / 10;   // tens
    memory[I + 2]   = value % 10;           // ones
}

// Fx55 - LD [I], Vx: Store registers V0 through Vx (inclusive) in memory starting at location I.
// The offset from I is increased by 1 for each value written, but I itself is left unmodified.
void Chip8::opcode_Fx55() {
    uint8_t rx = getX();
    for (int i = 0; i < rx + 1; ++i) memory[I + i] = V[i];
}

// Fx65 - LD Vx, [I]: Read registers V0 through Vx from memory starting at location I.
// The interpreter reads values from memory starting at location I into registers V0 through Vx.
void Chip8::opcode_Fx65() {
    uint8_t rx = getX();
    for (int i = 0; i < rx + 1; ++i) V[i] = memory[I + i];
}

// NONE - NOP: Invalid opcode
void Chip8::opcode_NONE() {
    std::cout << "Invalid opcode:   " << opcode << std::endl;
    exit(3);
}

/* Opcode Table Initialization */

void Chip8::tabulateOpcodes() {
    // The first digit of each opcode runs from 0x0 t0 0xF, hence sizeof(table) = 0xF + 1;
    table[0x0] = &Chip8::Table0;        // See (*) below

    table[0x1] = &Chip8::opcode_1nnn;
    table[0x2] = &Chip8::opcode_2nnn;
    table[0x3] = &Chip8::opcode_3xkk;
    table[0x4] = &Chip8::opcode_4xkk;
    table[0x5] = &Chip8::opcode_5xy0;
    table[0x6] = &Chip8::opcode_6xkk;
    table[0x7] = &Chip8::opcode_7xkk;

    table[0x8] = &Chip8::Table8;        // See (*) below

    table[0x9] = &Chip8::opcode_9xy0;
    table[0xA] = &Chip8::opcode_Annn;
    table[0xB] = &Chip8::opcode_Bnnn;
    table[0xC] = &Chip8::opcode_Cxkk;
    table[0xD] = &Chip8::opcode_Dxyn;

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
    table8[0x0] = &Chip8::opcode_8xy0;
    table8[0x1] = &Chip8::opcode_8yx1;
    table8[0x2] = &Chip8::opcode_8xy2;
    table8[0x3] = &Chip8::opcode_8xy3;
    table8[0x4] = &Chip8::opcode_8xy4;
    table8[0x5] = &Chip8::opcode_8xy5;
    table8[0x6] = &Chip8::opcode_8xy6;
    table8[0x7] = &Chip8::opcode_8xy7;
    table8[0xE] = &Chip8::opcode_8xyE;

    // $E needs an array that can index up to $E+1
    tableE[0x1] = &Chip8::opcode_ExA1;
    tableE[0xE] = &Chip8::opcode_Ex9E;

    // $F needs an array that can index up to $65+1
    for (Opcode& f : tableF) f = &Chip8::opcode_NONE;
    tableF[0x07] = &Chip8::opcode_Fx07;
    tableF[0x0A] = &Chip8::opcode_Fx0A;
    tableF[0x15] = &Chip8::opcode_Fx15;
    tableF[0x18] = &Chip8::opcode_Fx18;
    tableF[0x1E] = &Chip8::opcode_Fx1E;
    tableF[0x29] = &Chip8::opcode_Fx29;
    tableF[0x33] = &Chip8::opcode_Fx33;
    tableF[0x55] = &Chip8::opcode_Fx55;
    tableF[0x65] = &Chip8::opcode_Fx65;
}


/* Opcode Retrieval */

void Chip8::Table0() { (this->*table0[opcode & 0x000F])(); }

void Chip8::Table8() { (this->*table8[opcode & 0x000F])(); }

void Chip8::TableE() { (this->*tableE[opcode & 0x000F])(); }

void Chip8::TableF() { (this->*tableF[opcode & 0x00FF])(); }

