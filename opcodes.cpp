#include "chip8.h"
#include <cstdlib>  // for rand()


/* Display Opcodes */

// Clear the screen
void Chip8::opcode_00E0() {
    memset(display, 0, sizeof(display));
    pc += 2;
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
                if (screenPixel) {
                    V[0xF] = 1;
                }
            }

            *screenPixel ^= 1;
        }
    }
    pc += 2;
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
void Chip8::opcode_BNNN() {
    uint16_t address = opcode & 0x0FFF;
    pc = address + V[0];
}


/* Conditional Opcodes */

// Skip next instruction if VX equals NN
void Chip8::opcode_3XNN() {
    if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) {
        pc += 4;
    } else {
        pc += 2;
    }
}

// Skip next instruction if VX doesn't equal NN
void Chip8::opcode_4XNN() {
    if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) {
        pc += 4;
    } else {
        pc += 2;
    }
}

// Skip next instruction if VX equals VY
void Chip8::opcode_5XY0() {
    if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4]) {
        pc += 4;
    } else {
        pc += 2;
    }
}

// Skip next instruction if VX doesn't equal VY
void Chip8::opcode_9XY0() {
    if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4]) {
        pc += 4;
    } else {
        pc += 2;
    }
}

// Skip next instruction if key with value VX is pressed
void Chip8::opcode_EX9E() {
    if (key[V[(opcode & 0x0F00) >> 8]]) {
        pc += 4;
    } else {
        pc += 2;
    }
}

// Skip next instruction if key with value VX is not pressed
void Chip8::opcode_EXA1() {
    if (!key[V[(opcode & 0x0F00) >> 8]]) {
        pc += 4;
    } else {
        pc += 2;
    }
}


/* Arithmetic Opcodes */

// Set VX to NN
void Chip8::opcode_6XNN() {
    V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
    pc += 2;
}

// Adds NN to VX (carry flag is not changed).
void Chip8::opcode_7XNN() {
    V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
    pc += 2;
}

// Sets VX to the value of VY.
void Chip8::opcode_8XY0() {
    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
    pc += 2;
}

// Sets VX to VX or VY. (bitwise OR operation)
void Chip8::opcode_8XY1() {
    V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
    pc += 2;
}

// Sets VX to VX and VY. (bitwise AND operation)
void Chip8::opcode_8XY2() {
    V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
    pc += 2;
}

// Set VX to VX xor VY
void Chip8::opcode_8XY3() {
    V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
    pc += 2;
}

// Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there is not.
void Chip8::opcode_8XY4() {
    uint8_t rx = (opcode & 0x0F00) >> 8;
    uint8_t ry = (opcode & 0x00F0) >> 4;

    uint16_t sum = V[rx] + V[ry];

    V[0xF] = sum > 0xFF ? 1 : 0;

    V[rx] = sum & 0xFF;

    pc += 2;
}

// VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there is not.
void Chip8::opcode_8XY5() {
    uint8_t rx = (opcode & 0x0F00) >> 8;
    uint8_t ry = (opcode & 0x00F0) >> 4;

    V[0xF] = V[rx] > V[ry] ? 1 : 0;

    V[rx] -= V[ry];

    pc += 2;
}

// Stores the least significant bit of VX in VF and then shifts VX to the right by 1.
void Chip8::opcode_8XY6() {
    uint8_t rx = (opcode & 0x0F00) >> 8;
    V[0xF] = V[rx] & 0x1;
    V[rx] >>= 1;
    pc += 2;
}

// Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there is not.
void Chip8::opcode_8XY7() {
    uint8_t rx = (opcode & 0x0F00) >> 8;
    uint8_t ry = (opcode & 0x00F0) >> 4;

    V[0xF] = V[ry] > V[rx] ? 1 : 0;

    V[rx] = V[ry] - V[rx];

    pc += 2;
}

// Stores the most significant bit of VX in VF and then shifts VX to the left by 1.
void Chip8::opcode_8XYE() {
    uint8_t rx = (opcode & 0x0F00) >> 8;

    V[0xF] = (V[rx] & 0x80) >> 7;

    V[rx] <<= 1;

    pc += 2;
}


/* Random Opcodes */

// Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN.
void Chip8::opcode_CXNN() {
    V[(opcode & 0x0F00) >> 8] = (rand() & 256) & (opcode & 0x00FF);
    pc += 2;
}


/* Timer Opcodes */

// Set VX to the value of the delay timer.
void Chip8::opcode_FX07() {
    V[(opcode & 0x0F00) >> 8] = delay_timer;
    pc += 2;
}

// A key press is awaited, and then stored in VX (blocking operation, all instruction halted until next key event).
void Chip8::opcode_FX0A() {
    // TODO
}

// Sets the delay timer to VX.
void Chip8::opcode_FX15() {
    delay_timer = V[(opcode & 0x0F00) >> 8];
    pc += 2;
}

// Sets the sound timer to VX.
void Chip8::opcode_FX18() {
    sound_timer = V[(opcode & 0x0F00) >> 8];
    pc += 2;
}

// Adds VX to I. VF is not affected.
void Chip8::opcode_FX1E() {
    I += V[(opcode & 0x0F00) >> 8];
    pc += 2;
}


/* Memory Opcodes */

// Sets I to the address NNN.
void Chip8::opcode_ANNN() {
    I = opcode & 0x0FFF;
    pc += 2;
}

// Set I to the location of the sprite for character in VX
void Chip8::opcode_FX29() {

}

// Stores the binary-coded decimal (BCD) representation of VX,
// with the hundreds digit in memory at location in I,
// the tens digit at location I+1, and the ones digit at location I+2.
void Chip8::opcode_FX33() {

}

// Stores from V0 to VX (including VX) in memory, starting at address I.
// The offset from I is increased by 1 for each value written, but I itself is left unmodified.
void Chip8::opcode_FX55() {

}

// Fills from V0 to VX (including VX) with values from memory, starting at address I.
// The offset from I is increased by 1 for each value read, but I itself is left unmodified.
void Chip8::opcode_FX65() {

}












