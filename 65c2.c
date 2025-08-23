// This is a 65o2 emulator.
// Code it for fun :)
// Alpha_00, unfinished

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MEMORY_SIZE 0x10000
uint8_t MEMORY[MEMORY_SIZE];


#define NIL_OP {NOP, NONE}
// NVUBDIZC
#define C      (1 << 0) // Carry Flag
#define Z      (1 << 1) // Zero Flag
#define I      (1 << 2) // Interrupt Disable Flag
#define D      (1 << 3) // Decimal Mode Flag
#define B      (1 << 4) // Break Command Flag
#define U      (1 << 5) // Unused Flag (always 1)
#define V      (1 << 6) // Overflow Flag
#define N      (1 << 7) // Negative Flag



// AddrMode (Copy from ObaraEmmanuel's NES Project, thank you:))
typedef enum{
    NONE,       // Not a valid mode
    IMPL,       // Implicit operand
    ACC,        // Accumulator
    REL,        // Relative branch
    IMT,        // Immediate constant
    ZPG,        // Zero page
    ZPG_X,      // Zero page + X
    ZPG_Y,      // Zero page + Y
    ABS,        // 16-bit address
    ABS_X,      // 16-bit address + X
    ABS_Y,      // 16-bit address + Y
    IND,        // Indirect address (JMP only)
    IND_IDX,    // Zero page pointer + Y
    IDX_IND,    // (Zero page + X) pointer
} AddrMode;



// Opcode (Copy from ObaraEmmanuel's NES Project, thank you:))
typedef enum {
    ADC, //add with carry
    AND, //and (with accumulator)
    ASL, //arithmetic shift left
    BCC, //branch on carry clear
    BCS, //branch on carry set
    BEQ, //branch on equal (zero set)
    BIT, //bit test
    BMI, //branch on minus (negative set)
    BNE, //branch on not equal (zero clear)
    BPL, //branch on plus (negative clear)
    BRK, //break / interrupt
    BVC, //branch on overflow clear
    BVS, //branch on overflow set
    CLC, //clear carry
    CLD, //clear decimal
    CLI, //clear interrupt disable
    CLV, //clear overflow
    CMP, //compare (with accumulator)
    CPX, //compare with X
    CPY, //compare with Y
    DEC, //decrement
    DEX, //decrement X
    DEY, //decrement Y
    EOR, //exclusive or (with accumulator)
    INC, //increment
    INX, //increment X
    INY, //increment Y
    JMP, //jump
    JSR, //jump subroutine
    LDA, //load accumulator
    LDX, //load X
    LDY, //load Y
    LSR, //logical shift right
    NOP, //no operation
    ORA, //or with accumulator
    PHA, //push accumulator
    PHP, //push processor status (SR)
    PLA, //pull accumulator
    PLP, //pull processor status (SR)
    ROL, //rotate left
    ROR, //rotate right
    RTI, //return from interrupt
    RTS, //return from subroutine
    SBC, //subtract with carry
    SEC, //set carry
    SED, //set decimal
    SEI, //set interrupt disable
    STA, //store accumulator
    STX, //store X
    STY, //store Y
    TAX, //transfer accumulator to X
    TAY, //transfer accumulator to Y
    TSX, //transfer stack pointer to X
    TXA, //transfer X to accumulator
    TXS, //transfer X to stack pointer
    TYA, //transfer Y to accumulator

    // unofficial

    ALR,
    ANC,
    ARR,
    AXS,
    LAX,
    LAS,
    SAX,
    SHY,
    SHX,

    DCP,
    ISB,
    RLA,
    RRA,
    SLO,
    SRE,

    SKB,
    IGN,

} Opcode;

typedef struct {
    Opcode opcode;
    AddrMode mode;
} Instruction;

static const Instruction instructionLookup[256] =
{
//  HI\LO        0x0          0x1          0x2             0x3           0x4             0x5         0x6           0x7           0x8           0x9           0xA        0xB            0xC             0xD          0xE           0xF
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*  0x0  */  {BRK, IMPL},{ORA, IDX_IND}, NIL_OP,     {SLO, IDX_IND}, {NOP, ZPG},   {ORA, ZPG},   {ASL, ZPG},   {SLO, ZPG},   {PHP, IMPL}, {ORA, IMT},   {ASL, ACC},  {ANC, IMT},   {NOP, ABS},   {ORA, ABS},   {ASL, ABS},   {SLO, ABS},
/*  0x1  */  {BPL, REL}, {ORA, IND_IDX}, NIL_OP,     {SLO, IND_IDX}, {NOP, ZPG_X}, {ORA, ZPG_X}, {ASL, ZPG_X}, {SLO, ZPG_X}, {CLC, IMPL}, {ORA, ABS_Y}, {NOP, IMPL}, {SLO, ABS_Y}, {NOP, ABS_X}, {ORA, ABS_X}, {ASL, ABS_X}, {SLO, ABS_X},
/*  0x2  */  {JSR, ABS}, {AND, IDX_IND}, NIL_OP,     {RLA, IDX_IND}, {BIT, ZPG},   {AND, ZPG},   {ROL, ZPG},   {RLA, ZPG},   {PLP, IMPL}, {AND, IMT},   {ROL, ACC},  {ANC, IMT},   {BIT, ABS},   {AND, ABS},   {ROL, ABS},   {RLA, ABS},
/*  0x3  */  {BMI, REL}, {AND, IND_IDX}, NIL_OP,     {RLA, IND_IDX}, {NOP, ZPG_X}, {AND, ZPG_X}, {ROL, ZPG_X}, {RLA, ZPG_X}, {SEC, IMPL}, {AND, ABS_Y}, {NOP, IMPL}, {RLA, ABS_Y}, {NOP, ABS_X}, {AND, ABS_X}, {ROL, ABS_X}, {RLA, ABS_X},
/*  0x4  */  {RTI, IMPL},{EOR, IDX_IND}, NIL_OP,     {SRE, IDX_IND}, {NOP, ZPG},   {EOR, ZPG},   {LSR, ZPG},   {SRE, ZPG},   {PHA, IMPL}, {EOR, IMT},   {LSR, ACC},  {ALR, IMT},   {JMP, ABS},   {EOR, ABS},   {LSR, ABS},   {SRE, ABS},
/*  0x5  */  {BVC, REL}, {EOR, IND_IDX}, NIL_OP,     {SRE, IND_IDX}, {NOP, ZPG_X}, {EOR, ZPG_X}, {LSR, ZPG_X}, {SRE, ZPG_X}, {CLI, IMPL}, {EOR, ABS_Y}, {NOP, IMPL}, {SRE, ABS_Y}, {NOP, ABS_X}, {EOR, ABS_X}, {LSR, ABS_X}, {SRE, ABS_X},
/*  0x6  */  {RTS, IMPL},{ADC, IDX_IND}, NIL_OP,     {RRA, IDX_IND}, {NOP, ZPG},   {ADC, ZPG},   {ROR, ZPG},   {RRA, ZPG},   {PLA, IMPL}, {ADC, IMT},   {ROR, ACC},  {ARR, IMT},   {JMP, IND},   {ADC, ABS},   {ROR, ABS},   {RRA, ABS},
/*  0x7  */  {BVS, REL}, {ADC, IND_IDX}, NIL_OP,     {RRA, IND_IDX}, {NOP, ZPG_X}, {ADC, ZPG_X}, {ROR, ZPG_X}, {RRA, ZPG_X}, {SEI, IMPL}, {ADC, ABS_Y}, {NOP, IMPL}, {RRA, ABS_Y}, {NOP, ABS_X}, {ADC, ABS_X}, {ROR, ABS_X}, {RRA, ABS_X},
/*  0x8  */  {NOP, IMT}, {STA, IDX_IND}, {NOP, IMT}, {SAX, IDX_IND}, {STY, ZPG},   {STA, ZPG},   {STX, ZPG},   {SAX, ZPG},   {DEY, IMPL}, {NOP, IMT},   {TXA, IMPL}, {NOP, IMT},   {STY, ABS},   {STA, ABS},   {STX, ABS},   {SAX, ABS},
/*  0x9  */  {BCC, REL}, {STA, IND_IDX}, NIL_OP,     {NOP, IND_IDX}, {STY, ZPG_X}, {STA, ZPG_X}, {STX, ZPG_Y}, {SAX, ZPG_Y}, {TYA, IMPL}, {STA, ABS_Y}, {TXS, IMPL}, {NOP, ABS_Y}, {SHY, ABS_X}, {STA, ABS_X}, {SHX, ABS_Y}, {NOP, ABS_Y},
/*  0xA  */  {LDY, IMT}, {LDA, IDX_IND}, {LDX, IMT}, {LAX, IDX_IND}, {LDY, ZPG},   {LDA, ZPG},   {LDX, ZPG},   {LAX, ZPG},   {TAY, IMPL}, {LDA, IMT},   {TAX, IMPL}, {LAX, IMT},   {LDY, ABS},   {LDA, ABS},   {LDX, ABS},   {LAX, ABS},
/*  0xB  */  {BCS, REL}, {LDA, IND_IDX}, NIL_OP,     {LAX, IND_IDX}, {LDY, ZPG_X}, {LDA, ZPG_X}, {LDX, ZPG_Y}, {LAX, ZPG_Y}, {CLV, IMPL}, {LDA, ABS_Y}, {TSX, IMPL}, {LAS, ABS_Y}, {LDY, ABS_X}, {LDA, ABS_X}, {LDX, ABS_Y}, {LAX, ABS_Y},
/*  0xC  */  {CPY, IMT}, {CMP, IDX_IND}, {NOP, IMT}, {DCP, IDX_IND}, {CPY, ZPG},   {CMP, ZPG},   {DEC, ZPG},   {DCP, ZPG},   {INY, IMPL}, {CMP, IMT},   {DEX, IMPL}, {AXS, IMT},   {CPY, ABS},   {CMP, ABS},   {DEC, ABS},   {DCP, ABS},
/*  0xD  */  {BNE, REL}, {CMP, IND_IDX}, NIL_OP,     {DCP, IND_IDX}, {NOP, ZPG_X}, {CMP, ZPG_X}, {DEC, ZPG_X}, {DCP, ZPG_X}, {CLD, IMPL}, {CMP, ABS_Y}, {NOP, IMPL}, {DCP, ABS_Y}, {NOP, ABS_X}, {CMP, ABS_X}, {DEC, ABS_X}, {DCP, ABS_X},
/*  0xE  */  {CPX, IMT}, {SBC, IDX_IND}, {NOP, IMT}, {ISB, IDX_IND}, {CPX, ZPG},   {SBC, ZPG},   {INC, ZPG},   {ISB, ZPG},   {INX, IMPL}, {SBC, IMT},   NIL_OP,      {SBC, IMT},   {CPX, ABS},   {SBC, ABS},   {INC, ABS},   {ISB, ABS},
/*  0xF  */  {BEQ, REL}, {SBC, IND_IDX}, NIL_OP,     {ISB, IND_IDX}, {NOP, ZPG_X}, {SBC, ZPG_X}, {INC, ZPG_X}, {ISB, ZPG_X}, {SED, IMPL}, {SBC, ABS_Y}, {NOP, IMPL}, {ISB, ABS_Y}, {NOP, ABS_X}, {SBC, ABS_X}, {INC, ABS_X}, {ISB, ABS_X}
};


static const uint8_t cycles_table_frozen[256] = {
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



// Help Function Declaration
static void F_SET(MOS65o2 *cpu, uint8_t flag) {
  cpu -> P |= flag;
}

static void F_CLR(MOS65o2 *cpu, uint8_t flag) {
  cpu -> P &= ~ flag;
}

static uint8_t F_CHK(MOS65o2 *cpu, uint8_t flag) { 
  return (cpu -> P & flag) ? 1 : 0;
}


static uint8_t READ(uint16_t addr) {
  return MEMORY[addr];
}

static void WRITE(uint16_t addr, uint8_t value) {
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


int main() {
    MOS65o2 cpu;
    init_65o2(&cpu);
        
    return 0;
}

