// This is a 65o2 emulator.
// Code it for fun :)
// Alpha_00, unfinished

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MEMORY_SIZE 0x10000
uint8_t MEMORY[MEMORY_SIZE];
// NVUBDIZC
#define C      (1 << 0) // Carry Flag
#define Z      (1 << 1) // Zero Flag
#define I      (1 << 2) // Interrupt Disable Flag
#define D      (1 << 3) // Decimal Mode Flag
#define B      (1 << 4) // Break Command Flag
#define U      (1 << 5) // Unused Flag (always 1)
#define V      (1 << 6) // Overflow Flag
#define N      (1 << 7) // Negative Flag

const uint8_t cycles_table[256] = {
// This table lists the base number of clock cycles each 65o2 instruction takes to execute.
//  0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
    7,   6,   2,   8,   3,   3,   5,   5,   3,   2,   2,   2,   4,   4,   6,   6, // 0x0_
    2,   5,   2,   8,   4,   4,   6,   6,   2,   4,   2,   7,   4,   4,   7,   7, // 0x1_
    6,   6,   2,   8,   3,   3,   5,   5,   4,   2,   2,   2,   4,   4,   6,   6, // 0x2_
    2,   5,   2,   8,   4,   4,   6,   6,   2,   4,   2,   7,   4,   4,   7,   7, // 0x3_
    6,   6,   2,   8,   3,   3,   5,   5,   3,   2,   2,   2,   4,   4,   6,   6, // 0x4_
    2,   5,   2,   8,   4,   4,   6,   6,   2,   4,   2,   7,   4,   4,   7,   7, // 0x5_
    6,   6,   2,   8,   3,   3,   5,   5,   4,   2,   2,   2,   4,   4,   6,   6, // 0x6_
    2,   5,   2,   8,   4,   4,   6,   6,   2,   4,   2,   7,   4,   4,   7,   7, // 0x7_
    2,   6,   2,   6,   3,   3,   3,   3,   2,   2,   2,   2,   4,   4,   4,   4, // 0x8_
    2,   6,   2,   6,   4,   4,   4,   4,   2,   5,   2,   5,   5,   5,   5,   5, // 0x9_
    2,   6,   2,   6,   3,   3,   3,   3,   2,   2,   2,   2,   4,   4,   4,   4, // 0xA_
    2,   5,   2,   5,   4,   4,   4,   4,   2,   4,   2,   4,   4,   4,   4,   4, // 0xB_
    2,   6,   2,   8,   3,   3,   5,   5,   2,   2,   2,   2,   4,   4,   6,   6, // 0xC_
    2,   5,   2,   8,   4,   4,   6,   6,   2,   4,   2,   7,   4,   4,   7,   7, // 0xD_
    2,   6,   2,   8,   3,   3,   5,   5,   2,   2,   2,   2,   4,   4,   6,   6, // 0xE_
    2,   5,   2,   8,   4,   4,   6,   6,   2,   4,   2,   7,   4,   4,   7,   7  // 0xF_
};

typedef struct MOS65o2 {
  uint8_t    A;     // Accumulator               - A
  uint8_t    X;     // X Index Register          - X
  uint8_t    Y;     // Y Index Register          - Y
  uint8_t   SP;     // Stack Pointer             - SP
  uint8_t    P;     // Processor Status Register - P
  uint16_t  PC;     // Program Counter           - PC
  uint32_t CLC;     // Total time cycles of 65o2 - CYC
} MOS65o2;


void F_SET(MOS65o2 *cpu, uint8_t flag) {
  cpu -> P |= flag;
}

void F_CLR(MOS65o2 *cpu, uint8_t flag) {
  cpu -> P &= ~ flag;
}

uint8_t F_CHK(MOS65o2 *cpu, uint8_t flag) { 
  return (cpu -> P & flag) ? 1 : 0;
}


uint8_t READ(uint16_t addr) {
  return MEMORY[addr];
}

void WRITE(uint16_t addr, uint8_t value) {
  MEMORY[addr] = value;
}


void init_65o2(MOS65o2 *cpu) {
  // Initialize Registers.
  cpu -> A  = 0;
  cpu -> X  = 0;
  cpu -> Y  = 0;
  cpu -> SP = 0xFF; // Stack Pointer initialized to point to $01FF.
  cpu -> P  = U | I;
  cpu -> PC = 0xC000;
  // Initialize Clock Cycles.
  cpu -> CLC = 0;
  // Initialize Memory.
  for (int i = 0; i < MEMORY_SIZE; i++)
    MEMORY[i] = 0x00;
}

uint8_t EXE(MOS65o2 *cpu) {
    uint16_t    _PC = cpu -> PC;
    
    uint8_t  opcode = READ(cpu -> PC++);
    uint8_t  cycles = cycles_table[opcode];
    
    printf("Executing PC: 0x%04X, Opcode: 0x%02X\n", _PC, opcode);
    
    switch (opcode) {
        case 0xA9: {    // LDA #byte (Immediate)
            uint8_t value = READ(cpu -> PC++);
            cpu -> A = value;
            
            if (cpu -> A == 0) F_SET(cpu, Z); else F_CLR(cpu, Z);
            if (cpu -> A & 0x80) F_SET(cpu, N); else F_CLR(cpu, N);
            
            printf("LDA #$02X -> A = 0x%02X\n", value, cpu -> A);
            
            return(cycles);
        }
            
        case 0x8D: {    // STA addr (Absolute)
            uint8_t   low_byte = READ(cpu -> PC++);
            uint8_t  high_byte = READ(cpu -> PC++);
            
            uint16_t addr = (high_byte << 8) | low_byte;
            uint8_t value = READ(addr);
            WRITE(addr, cpu -> A);
            
            printf("STA #$02X -> A = 0x%02X\n", value, cpu -> A);
            
            return(cycles);
        }
            
        case 0xBD: {    // LDA abs, X (ABsolute, X)
            uint8_t   low_byte = READ(cpu -> PC++);
            uint8_t  high_byte = READ(cpu -> PC++);
            
            uint16_t base_addr = (high_byte << 8) | low_byte;
            uint16_t effe_addr = base_addr + cpu -> X;
            
            if ((base_addr & 0xFF00) != (effe_addr & 0xFF00)) cycles ++;
            
            cpu -> A = READ(effe_addr);
            if (cpu -> A == 0) F_SET(cpu, Z); else F_CLR(cpu, Z);
            if (cpu -> A & 0x80) F_SET(cpu, N); else F_CLR(cpu, N);
            printf("LDA $0x%04X,X -> A = 0x%02X (Effective: 0x%04X)\n", base_addr, cpu->A, effe_addr);

            break;
        }
    }
    
    return 0;
}

void UI(MOS65o2 *cpu) {
    printf(".\n");
    printf("|── Register\n");
    printf("|   └──  A: 0x%02X\n", cpu -> A);
    printf("|   └──  X: 0x%02X\n", cpu -> X);
    printf("|   └──  Y: 0x%02X\n", cpu -> Y);
    printf("|   └── SP: 0x%02X\n", cpu -> SP);
    printf("|   └──  P: 0x%02X\n", cpu -> P);
    printf("|   └── PC: 0x%04X\n", cpu -> PC);
    printf("|── Clock Cycle\n");
    printf("    └──CLC: 0x%04X\n\n\n", cpu -> CLC);
}

int main() {
    MOS65o2 cpu;
    init_65o2(&cpu);
    
    // Program.
    MEMORY[0xC000] = 0xA9;
    MEMORY[0xC001] = 0x07;
    printf("\n\n$Emulator - Starting Emulation.\n");
    // Cycle Start.
    
    for (int i = 0; i < 10; i++) {
        uint8_t _cycle =  EXE(&cpu);
        if (_cycle == 0) {
            cpu.CLC ++;
        } else {
            cpu.CLC += _cycle;
        }
        
        UI(&cpu);
        
        if (cpu.PC >= MEMORY_SIZE) {
            printf("$Emulator - PC out of bounds. Emulation halted.\n");
            break;
        }
    }
    
    printf("$Emulator - Emulation finished.\n");
    
    
    
    return 0;
}
