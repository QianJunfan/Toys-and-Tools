// This is a project I wrote while on vacation in Zhejiang, China.
// It was a real challenge, but coding is always fun, isn't it?
// In the future, I might use this CPU state machine emulator in my Apple II emulator project.
//
//                                        ~ My home page : https://github.com/QianJunfan
//                                        ~ Hope you enjoying coding :)
//
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* MARK: 65o2 Emulator in C */
// NVUBDIZC
#define C      (1 << 0) // carry flag
#define Z      (1 << 1) // zero flag
#define I      (1 << 2) // interrupt disable flag
#define D      (1 << 3) // decimal mode flag
#define B      (1 << 4) // break command flag
#define U      (1 << 5) // unused flag (always 1)
#define V      (1 << 6) // overflow flag
#define N      (1 << 7) // negative flag

#define NIL_OP {NOP, NONE}

#define MEMORY_SIZE 0x10000

uint8_t MEMORY[MEMORY_SIZE];

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
typedef enum {
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
} Addrmode;
typedef enum {
    ITRP_NMI,
    ITRP_IRQ,
    ITRP_BRK
} Itrp;
typedef struct Isc {
      Opcode   opcode;
      Addrmode mode;
} Isc;

// Instration look up (Copy from ObaraEmmanuel's NES Project, thank you again for making such a great datasheet :))
//    Link - https://github.com/ObaraEmmanuel/NES
// Thank you again :)
static const Isc     isc_lookup[256] = {
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
static const uint8_t cyc_lookup[256] = {
// HI/LO 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
// -------------------------------------------------------
/* 0 */  7, 6, 0, 8, 3, 3, 5, 5, 3, 2, 2, 2, 4, 4, 6, 6,
/* 1 */  2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
/* 2 */  6, 6, 0, 8, 3, 3, 5, 5, 4, 2, 2, 2, 4, 4, 6, 6,
/* 3 */  2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
/* 4 */  6, 6, 0, 8, 3, 3, 5, 5, 3, 2, 2, 2, 3, 4, 6, 6,
/* 5 */  2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
/* 6 */  6, 6, 0, 8, 3, 3, 5, 5, 4, 2, 2, 2, 5, 4, 6, 6,
/* 7 */  2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
/* 8 */  2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
/* 9 */  2, 6, 0, 6, 4, 4, 4, 4, 2, 5, 2, 5, 5, 5, 5, 5,
/* A */  2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
/* B */  2, 5, 0, 5, 4, 4, 4, 4, 2, 4, 2, 4, 4, 4, 4, 4,
/* C */  2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
/* D */  2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
/* E */  2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
/* F */  2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
};

typedef struct MOS65o2 {
      uint8_t    A;     // accumulator               - A
      uint8_t    X;     // x Index Register          - X
      uint8_t    Y;     // y Index Register          - Y
      uint8_t   SP;     // stack Pointer             - SP
      uint8_t    P;     // processor Status Register - P
      uint16_t  PC;     // program Counter           - PC
      uint32_t CLC;     // total time cycles         - CYC
      
      bool NMI;
      bool IRQ;
} MOS65o2;

// helper Functions
static void    F_SET(MOS65o2 *cpu, uint8_t flag) {cpu -> P |= flag;}
static void    F_CLR(MOS65o2 *cpu, uint8_t flag) {cpu -> P &= ~ flag;}
static uint8_t F_CHK(MOS65o2 *cpu, uint8_t flag) {return (cpu -> P & flag) ? 1 : 0;}
static void    WRITE(uint16_t addr, uint8_t value) {MEMORY[addr] = value;}
static uint8_t READ (uint16_t addr) {return MEMORY[addr];}

// api
void init_65o2(MOS65o2 *cpu);
void rset_65o2(MOS65o2 *cpu);
void itrp_65o2(MOS65o2 *cpu, Itrp type);
void exec_65o2(MOS65o2 *cpu, int clc_to_run);


void init_65o2(MOS65o2 *cpu) {
      // init registers
      cpu -> A  = 0;
      cpu -> X  = 0;
      cpu -> Y  = 0;
      cpu -> SP = 0xFF; // stack Pointer initialized to point to $01FF.
      cpu -> P  = U | I;
      cpu -> PC = 0xC000;
      // init clock cycles
      cpu -> CLC = 0;
      // init memory
      for (int i = 0; i < MEMORY_SIZE; i++)
            MEMORY[i] = 0x00;
      // init itrp
      cpu -> IRQ = false;
      cpu -> NMI = false;
}


void rset_65o2(MOS65o2 *cpu) {
      // reset vector
      cpu -> PC = (READ(0xFFFC) | READ(0xFFFD) << 8);
      // reset registers
      cpu -> A  = 0;
      cpu -> X  = 0;
      cpu -> Y  = 0;
      cpu -> SP = 0xFF;
      cpu -> P  = I | U;
      // reset time cycle
      cpu -> CLC = 0;
      // reset itrp
      cpu -> IRQ = false;
      cpu -> NMI = false;
}
void itrp_65o2(MOS65o2 *cpu, Itrp type) {
      uint16_t vector;
      uint16_t pc_to_push = cpu->PC;
      bool b_flag_state = false;
      
      switch (type) {
            case ITRP_NMI:
                  vector = 0xFFFA;
                  pc_to_push = cpu->PC;
                  break;
                  
            case ITRP_IRQ:
                  if (F_CHK(cpu, I)) {
                        return;
                  }
                  vector     = 0xFFFE;
                  pc_to_push = cpu->PC;
                  
                  break;
                  
            case ITRP_BRK:
                  vector = 0xFFFE;
                  b_flag_state = true;
                  pc_to_push  = cpu->PC + 2;
                  break;
      }
      
      // push PC and P register to the stack
      WRITE(0x0100 + cpu -> SP--, (pc_to_push >> 8));
      WRITE(0x0100 + cpu -> SP--, (pc_to_push & 0xFF));
      
      uint8_t status_to_push = cpu->P;
      if (b_flag_state) {
            status_to_push |= B;
      } else {
            status_to_push &= ~B;
      }
      WRITE(0x0100 + cpu->SP--, status_to_push);
      
      F_SET(cpu, I);
      cpu -> PC = READ(vector) | (READ(vector + 1) << 8);
      
      // interrupt handling consumes 7 cycles
      cpu -> CLC += 7;
}
// calc clock cycles & exec commands
static void cccec(MOS65o2 *cpu) {
      // fetch opcode
      uint8_t opcode = READ(cpu->PC);
      
      // decode opcode
      Isc isc = isc_lookup[opcode];
      
      if (isc.opcode == NOP && isc.mode == NONE) {
            // unimplemented or illegal opcode
            cpu->PC += 1;
            cpu->CLC += 2; // assume a default minimum cycle
            return;
      }
      
      // cycles
      uint8_t  pc_increment = 0;
      uint8_t  data = 0;
      uint16_t addr = 0;
      
      uint8_t  tmp_val;
      uint16_t tmp_addr;
      
      uint8_t lo;
      uint8_t hi;
      
      uint8_t page_cross_penalty = 0;
      
      switch (isc.mode) {
            case IMPL:
            case ACC:
                  pc_increment = 1;
                  break;
            case IMT:
                  data = READ(cpu->PC + 1);
                  pc_increment = 2;
                  break;
            case ZPG:
                  addr = READ(cpu->PC + 1);
                  pc_increment = 2;
                  break;
            case ZPG_X:
                  addr = (READ(cpu->PC + 1) + cpu->X) & 0xFF;
                  pc_increment = 2;
                  break;
            case ZPG_Y:
                  addr = (READ(cpu->PC + 1) + cpu->Y) & 0xFF;
                  pc_increment = 2;
                  break;
            case ABS:
                  addr = READ(cpu->PC + 1) | (READ(cpu->PC + 2) << 8);
                  pc_increment = 3;
                  break;
            case ABS_X:
                  tmp_addr = READ(cpu->PC + 1) | (READ(cpu->PC + 2) << 8);
                  addr = tmp_addr + cpu->X;
                  if ((addr & 0xFF00) != (tmp_addr & 0xFF00)) {
                        page_cross_penalty = 1; // page cross penalty for ABS_X
                  }
                  pc_increment = 3;
                  break;
            case ABS_Y:
                  tmp_addr = READ(cpu->PC + 1) | (READ(cpu->PC + 2) << 8);
                  addr = tmp_addr + cpu->Y;
                  if ((addr & 0xFF00) != (tmp_addr & 0xFF00)) {
                        page_cross_penalty = 1; // page cross penalty for ABS_Y
                  }
                  pc_increment = 3;
                  break;
            case IND:
                  // 6502 bug: JMP ($xxFF) wraps to ($xx00) instead of ($xx+1)00
                  lo = READ(cpu->PC + 1);
                  hi = READ(cpu->PC + 2);
                  tmp_addr = (hi << 8) | lo;
                  
                  addr = READ(tmp_addr) | (READ((hi << 8) | ((lo + 1) & 0xFF)) << 8);
                  pc_increment = 3;
                  break;
            case IDX_IND:
                  tmp_addr = (READ(cpu->PC + 1) + cpu->X) & 0xFF;
                  addr = READ(tmp_addr) | (READ((tmp_addr + 1) & 0xFF) << 8);
                  pc_increment = 2;
                  break;
            case IND_IDX:
                  tmp_addr = READ(cpu->PC + 1);
                  addr = (READ(tmp_addr) | (READ((tmp_addr + 1) & 0xFF) << 8)) + cpu->Y;
                  if ((addr & 0xFF00) != ((READ(tmp_addr) | (READ((tmp_addr + 1) & 0xFF) << 8)) & 0xFF00)) {
                        page_cross_penalty = 1;
                  }
                  pc_increment = 2;
                  break;
            case REL:
                  data = READ(cpu->PC + 1);
                  pc_increment = 2;
                  break;
            case NONE:
            default:
                  pc_increment = 1;
                  break;
      }
      
      switch (isc.opcode) {
                  // load instructions
            case LDA:
                  if (isc.mode == IMT) {
                        cpu->A = data;
                  } else {
                        cpu->A = READ(addr);
                  }
                  (cpu->A == 0) ? F_SET(cpu, Z) : F_CLR(cpu, Z);
                  (cpu->A & N) ? F_SET(cpu, N) : F_CLR(cpu, N);
                  break;
            case LDX:
                  if (isc.mode == IMT) {
                        cpu->X = data;
                  } else {
                        cpu->X = READ(addr);
                  }
                  (cpu->X == 0) ? F_SET(cpu, Z) : F_CLR(cpu, Z);
                  (cpu->X & N) ? F_SET(cpu, N) : F_CLR(cpu, N);
                  break;
            case LDY:
                  if (isc.mode == IMT) {
                        cpu->Y = data;
                  } else {
                        cpu->Y = READ(addr);
                  }
                  (cpu->Y == 0) ? F_SET(cpu, Z) : F_CLR(cpu, Z);
                  (cpu->Y & N) ? F_SET(cpu, N) : F_CLR(cpu, N);
                  break;
                  
                  // store instructions
            case STA:
                  WRITE(addr, cpu->A);
                  break;
            case STX:
                  WRITE(addr, cpu->X);
                  break;
            case STY:
                  WRITE(addr, cpu->Y);
                  break;
                  
                  // transfer instructions
            case TAX:
                  cpu->X = cpu->A;
                  (cpu->X == 0) ? F_SET(cpu, Z) : F_CLR(cpu, Z);
                  (cpu->X & N) ? F_SET(cpu, N) : F_CLR(cpu, N);
                  break;
            case TXA:
                  cpu->A = cpu->X;
                  (cpu->A == 0) ? F_SET(cpu, Z) : F_CLR(cpu, Z);
                  (cpu->A & N) ? F_SET(cpu, N) : F_CLR(cpu, N);
                  break;
            case TAY:
                  cpu->Y = cpu->A;
                  (cpu->Y == 0) ? F_SET(cpu, Z) : F_CLR(cpu, Z);
                  (cpu->Y & N) ? F_SET(cpu, N) : F_CLR(cpu, N);
                  break;
            case TYA:
                  cpu->A = cpu->Y;
                  (cpu->A == 0) ? F_SET(cpu, Z) : F_CLR(cpu, Z);
                  (cpu->A & N) ? F_SET(cpu, N) : F_CLR(cpu, N);
                  break;
            case TSX:
                  cpu->X = cpu->SP;
                  (cpu->X == 0) ? F_SET(cpu, Z) : F_CLR(cpu, Z);
                  (cpu->X & N) ? F_SET(cpu, N) : F_CLR(cpu, N);
                  break;
            case TXS:
                  cpu->SP = cpu->X;
                  break;
                  
                  // stack instructions
            case PHA:
                  WRITE(0x0100 + cpu->SP--, cpu->A);
                  break;
            case PLA:
                  cpu->A = READ(0x0100 + ++cpu->SP);
                  (cpu->A == 0) ? F_SET(cpu, Z) : F_CLR(cpu, Z);
                  (cpu->A & N) ? F_SET(cpu, N) : F_CLR(cpu, N);
                  break;
            case PHP:
                  WRITE(0x0100 + cpu->SP--, cpu->P | B);
                  break;
            case PLP:
                  cpu->P = READ(0x0100 + ++cpu->SP) | U;
                  break;
                  
                  // flag instructions
            case CLC: F_CLR(cpu, C); break;
            case CLD: F_CLR(cpu, D); break;
            case CLI: F_CLR(cpu, I); break;
            case CLV: F_CLR(cpu, V); break;
            case SEC: F_SET(cpu, C); break;
            case SED: F_SET(cpu, D); break;
            case SEI: F_SET(cpu, I); break;
                  
                  // arithmetic and logical instructions
            case ADC:
            case SBC:
            case AND:
            case EOR:
            case ORA:
            case CMP:
            case CPX:
            case CPY: {
                  uint8_t val;
                  if (isc.mode == IMT) {
                        val = data;
                  } else {
                        val = READ(addr);
                  }
                  
                  switch (isc.opcode) {
                        case ADC: {
                              if (F_CHK(cpu, D)) { // decimal mode
                                    uint8_t lo_nib = (cpu->A & 0x0F) + (val & 0x0F) + F_CHK(cpu, C);
                                    if (lo_nib > 0x09) {
                                          lo_nib += 0x06;
                                    }
                                    uint8_t hi_nib = (cpu->A >> 4) + (val >> 4) + (lo_nib > 0x0F);
                                    if (hi_nib > 0x09) {
                                          hi_nib += 0x60;
                                          F_SET(cpu, C);
                                    } else {
                                          F_CLR(cpu, C);
                                    }
                                    cpu->A = ((hi_nib << 4) | (lo_nib & 0x0F));
                              } else { // binary mode
                                    uint16_t result = cpu->A + val + F_CHK(cpu, C);
                                    F_CLR(cpu, V);
                                    if (((cpu->A ^ result) & (val ^ result) & 0x80) != 0) {
                                          F_SET(cpu, V);
                                    }
                                    (result > 0xFF) ? F_SET(cpu, C) : F_CLR(cpu, C);
                                    cpu->A = result & 0xFF;
                              }
                              (cpu->A == 0) ? F_SET(cpu, Z) : F_CLR(cpu, Z);
                              (cpu->A & N) ? F_SET(cpu, N) : F_CLR(cpu, N);
                              break;
                        }
                        case SBC: {
                              uint8_t val;
                              if (isc.mode == IMT) {
                                    val = data;
                              } else {
                                    val = READ(addr);
                              }
                              
                              if (F_CHK(cpu, D)) { // Decimal mode
                                    uint8_t oldA = cpu->A;
                                    uint16_t result = cpu->A - val - (1 - F_CHK(cpu, C));
                                    
                                    if (((oldA & 0x0F) - (val & 0x0F) - (1 - F_CHK(cpu, C))) & 0x10) {
                                          result -= 0x06;
                                    }
                                    
                                    if (result & 0x100) {
                                          result -= 0x60;
                                    }
                                    
                                    (result <= 0xFF) ? F_SET(cpu, C) : F_CLR(cpu, C);
                                    cpu->A = result & 0xFF;
                              } else { // Binary mode
                                    uint16_t result = cpu->A - val - (1 - F_CHK(cpu, C));
                                    F_CLR(cpu, V);
                                    if (((cpu->A ^ result) & (~val ^ result) & 0x80) != 0) {
                                          F_SET(cpu, V);
                                    }
                                    (result <= 0xFF) ? F_SET(cpu, C) : F_CLR(cpu, C);
                                    cpu->A = result & 0xFF;
                              }
                              
                              (cpu->A == 0) ? F_SET(cpu, Z) : F_CLR(cpu, Z);
                              (cpu->A & N) ? F_SET(cpu, N) : F_CLR(cpu, N);
                              break;
                        }
                        case AND:
                              cpu->A &= val;
                              (cpu->A == 0) ? F_SET(cpu, Z) : F_CLR(cpu, Z);
                              (cpu->A & N) ? F_SET(cpu, N) : F_CLR(cpu, N);
                              break;
                        case EOR:
                              cpu->A ^= val;
                              (cpu->A == 0) ? F_SET(cpu, Z) : F_CLR(cpu, Z);
                              (cpu->A & N) ? F_SET(cpu, N) : F_CLR(cpu, N);
                              break;
                        case ORA:
                              cpu->A |= val;
                              (cpu->A == 0) ? F_SET(cpu, Z) : F_CLR(cpu, Z);
                              (cpu->A & N) ? F_SET(cpu, N) : F_CLR(cpu, N);
                              break;
                        case CMP: {
                              uint16_t result = cpu->A - val;
                              (cpu->A >= val) ? F_SET(cpu, C) : F_CLR(cpu, C);
                              ((result & 0xFF) == 0) ? F_SET(cpu, Z) : F_CLR(cpu, Z);
                              ((result & 0xFF) & N) ? F_SET(cpu, N) : F_CLR(cpu, N);
                              break;
                        }
                        case CPX: {
                              uint16_t result = cpu->X - val;
                              (cpu->X >= val) ? F_SET(cpu, C) : F_CLR(cpu, C);
                              ((result & 0xFF) == 0) ? F_SET(cpu, Z) : F_CLR(cpu, Z);
                              ((result & 0xFF) & N) ? F_SET(cpu, N) : F_CLR(cpu, N);
                              break;
                        }
                        case CPY: {
                              uint16_t result = cpu->Y - val;
                              (cpu->Y >= val) ? F_SET(cpu, C) : F_CLR(cpu, C);
                              ((result & 0xFF) == 0) ? F_SET(cpu, Z) : F_CLR(cpu, Z);
                              ((result & 0xFF) & N) ? F_SET(cpu, N) : F_CLR(cpu, N);
                              break;
                        }
                        default: break;
                  }
                  break;
            }
            case BIT: {
                  tmp_val = READ(addr);
                  ((cpu->A & tmp_val) == 0) ? F_SET(cpu, Z) : F_CLR(cpu, Z);
                  (tmp_val & V) ? F_SET(cpu, V) : F_CLR(cpu, V);
                  (tmp_val & N) ? F_SET(cpu, N) : F_CLR(cpu, N);
                  break;
            }
                  // increment/decrement instructions
            case DEC: {
                  tmp_val = READ(addr);
                  tmp_val--;
                  WRITE(addr, tmp_val);
                  (tmp_val == 0) ? F_SET(cpu, Z) : F_CLR(cpu, Z);
                  (tmp_val & N) ? F_SET(cpu, N) : F_CLR(cpu, N);
                  break;
            }
            case DEX:
                  cpu->X--;
                  (cpu->X == 0) ? F_SET(cpu, Z) : F_CLR(cpu, Z);
                  (cpu->X & N) ? F_SET(cpu, N) : F_CLR(cpu, N);
                  break;
            case DEY:
                  cpu->Y--;
                  (cpu->Y == 0) ? F_SET(cpu, Z) : F_CLR(cpu, Z);
                  (cpu->Y & N) ? F_SET(cpu, N) : F_CLR(cpu, N);
                  break;
            case INC: {
                  tmp_val = READ(addr);
                  tmp_val++;
                  WRITE(addr, tmp_val);
                  (tmp_val == 0) ? F_SET(cpu, Z) : F_CLR(cpu, Z);
                  (tmp_val & N) ? F_SET(cpu, N) : F_CLR(cpu, N);
                  break;
            }
            case INX:
                  cpu->X++;
                  (cpu->X == 0) ? F_SET(cpu, Z) : F_CLR(cpu, Z);
                  (cpu->X & N) ? F_SET(cpu, N) : F_CLR(cpu, N);
                  break;
            case INY:
                  cpu->Y++;
                  (cpu->Y == 0) ? F_SET(cpu, Z) : F_CLR(cpu, Z);
                  (cpu->Y & N) ? F_SET(cpu, N) : F_CLR(cpu, N);
                  break;
                  
                  // shift/rotate instructions
            case ASL: {
                  tmp_val = (isc.mode == ACC) ? cpu->A : READ(addr);
                  (tmp_val & 0x80) ? F_SET(cpu, C) : F_CLR(cpu, C);
                  tmp_val <<= 1;
                  (isc.mode == ACC) ? (cpu->A = tmp_val) : WRITE(addr, tmp_val);
                  (tmp_val == 0) ? F_SET(cpu, Z) : F_CLR(cpu, Z);
                  (tmp_val & N) ? F_SET(cpu, N) : F_CLR(cpu, N);
                  break;
            }
            case LSR: {
                  tmp_val = (isc.mode == ACC) ? cpu->A : READ(addr);
                  (tmp_val & 0x01) ? F_SET(cpu, C) : F_CLR(cpu, C);
                  tmp_val >>= 1;
                  (isc.mode == ACC) ? (cpu->A = tmp_val) : WRITE(addr, tmp_val);
                  (tmp_val == 0) ? F_SET(cpu, Z) : F_CLR(cpu, Z);
                  (tmp_val & N) ? F_SET(cpu, N) : F_CLR(cpu, N);
                  break;
            }
            case ROL: {
                  tmp_val = (isc.mode == ACC) ? cpu->A : READ(addr);
                  uint8_t old_c = F_CHK(cpu, C);
                  (tmp_val & 0x80) ? F_SET(cpu, C) : F_CLR(cpu, C);
                  tmp_val = (tmp_val << 1) | old_c;
                  (isc.mode == ACC) ? (cpu->A = tmp_val) : WRITE(addr, tmp_val);
                  (tmp_val == 0) ? F_SET(cpu, Z) : F_CLR(cpu, Z);
                  (tmp_val & N) ? F_SET(cpu, N) : F_CLR(cpu, N);
                  break;
            }
            case ROR: {
                  tmp_val = (isc.mode == ACC) ? cpu->A : READ(addr);
                  uint8_t old_c = F_CHK(cpu, C) << 7;
                  (tmp_val & 0x01) ? F_SET(cpu, C) : F_CLR(cpu, C);
                  tmp_val = (tmp_val >> 1) | old_c;
                  (isc.mode == ACC) ? (cpu->A = tmp_val) : WRITE(addr, tmp_val);
                  (tmp_val == 0) ? F_SET(cpu, Z) : F_CLR(cpu, Z);
                  (tmp_val & N) ? F_SET(cpu, N) : F_CLR(cpu, N);
                  break;
            }
                  
                  // jump instructions
            case JMP:
                  cpu->PC = addr;
                  pc_increment = 0;
                  break;
                  // JSR
            case JSR: {
                  uint16_t ret_addr = cpu->PC + 2;
                  WRITE(0x0100 + cpu->SP--, (ret_addr >> 8));
                  WRITE(0x0100 + cpu->SP--, (ret_addr & 0xFF));
                  cpu->PC = addr;
                  pc_increment = 0;
                  break;
            }
                  // RTS
            case RTS: {
                  uint8_t lo = READ(0x0100 + ++cpu->SP);
                  uint8_t hi = READ(0x0100 + ++cpu->SP);
                  cpu->PC = ((uint16_t)hi << 8) | lo;
                  cpu->PC++;
                  pc_increment = 0;
                  break;
            }
            case RTI:
                  cpu->P = READ(0x0100 + ++cpu->SP) | U;
                  F_SET(cpu, U);
                  lo = READ(0x0100 + ++cpu->SP);
                  hi = READ(0x0100 + ++cpu->SP);
                  cpu->PC = (hi << 8) | lo;
                  pc_increment = 0;
                  break;
                  
                  // branch instructions
            case BCC:
                  if (!F_CHK(cpu, C)) {
                        tmp_addr = cpu->PC + pc_increment + (int8_t)data;
                        if ((cpu->PC + pc_increment) / 256 != tmp_addr / 256) {
                              cpu->CLC += 2; // cross page
                        } else {
                              cpu->CLC += 1; // same page
                        }
                        cpu->PC = tmp_addr;
                        pc_increment = 0;
                  }
                  break;
            case BCS:
                  if (F_CHK(cpu, C)) {
                        tmp_addr = cpu->PC + pc_increment + (int8_t)data;
                        if ((cpu->PC + pc_increment) / 256 != tmp_addr / 256) {
                              cpu->CLC += 2;
                        } else {
                              cpu->CLC += 1;
                        }
                        cpu->PC = tmp_addr;
                        pc_increment = 0;
                  }
                  break;
            case BEQ:
                  if (F_CHK(cpu, Z)) {
                        tmp_addr = cpu->PC + pc_increment + (int8_t)data;
                        if ((cpu->PC + pc_increment) / 256 != tmp_addr / 256) {
                              cpu->CLC += 2;
                        } else {
                              cpu->CLC += 1;
                        }
                        cpu->PC = tmp_addr;
                        pc_increment = 0;
                  }
                  break;
            case BMI:
                  if (F_CHK(cpu, N)) {
                        tmp_addr = cpu->PC + pc_increment + (int8_t)data;
                        if ((cpu->PC + pc_increment) / 256 != tmp_addr / 256) {
                              cpu->CLC += 2;
                        } else {
                              cpu->CLC += 1;
                        }
                        cpu->PC = tmp_addr;
                        pc_increment = 0;
                  }
                  break;
            case BNE:
                  if (!F_CHK(cpu, Z)) {
                        tmp_addr = cpu->PC + pc_increment + (int8_t)data;
                        if ((cpu->PC + pc_increment) / 256 != tmp_addr / 256) {
                              cpu->CLC += 2;
                        } else {
                              cpu->CLC += 1;
                        }
                        cpu->PC = tmp_addr;
                        pc_increment = 0;
                  }
                  break;
            case BPL:
                  if (!F_CHK(cpu, N)) {
                        tmp_addr = cpu->PC + pc_increment + (int8_t)data;
                        if ((cpu->PC + pc_increment) / 256 != tmp_addr / 256) {
                              cpu->CLC += 2;
                        } else {
                              cpu->CLC += 1;
                        }
                        cpu->PC = tmp_addr;
                        pc_increment = 0;
                  }
                  break;
            case BVC:
                  if (!F_CHK(cpu, V)) {
                        tmp_addr = cpu->PC + pc_increment + (int8_t)data;
                        if ((cpu->PC + pc_increment) / 256 != tmp_addr / 256) {
                              cpu->CLC += 2;
                        } else {
                              cpu->CLC += 1;
                        }
                        cpu->PC = tmp_addr;
                        pc_increment = 0;
                  }
                  break;
            case BVS:
                  if (F_CHK(cpu, V)) {
                        tmp_addr = cpu->PC + pc_increment + (int8_t)data;
                        if ((cpu->PC + pc_increment) / 256 != tmp_addr / 256) {
                              cpu->CLC += 2;
                        } else {
                              cpu->CLC += 1;
                        }
                        cpu->PC = tmp_addr;
                        pc_increment = 0;
                  }
                  break;
            case BRK:
                  itrp_65o2(cpu, ITRP_BRK);
                  pc_increment = 0; // the brk handler advances the pc
                  break;
                  
                  // unofficial opcodes - simplified implementation based on common behavior
            case NOP: break;
            case SLO:
            case RLA:
            case SRE:
            case RRA:
            case DCP:
            case ISB: {
                  uint8_t val;
                  if (isc.mode == IDX_IND || isc.mode == IND_IDX) {
                        page_cross_penalty = 0; // these unofficial opcodes don't have a page cross penalty
                  } else if (isc.mode == ABS_X || isc.mode == ABS_Y) {
                        page_cross_penalty = 0;
                  }
                  val = READ(addr);
                  
                  switch (isc.opcode) {
                        case SLO: // ASL + ORA
                              (val & 0x80) ? F_SET(cpu, C) : F_CLR(cpu, C);
                              val <<= 1;
                              WRITE(addr, val);
                              cpu->A |= val;
                              (cpu->A == 0) ? F_SET(cpu, Z) : F_CLR(cpu, Z);
                              (cpu->A & N) ? F_SET(cpu, N) : F_CLR(cpu, N);
                              break;
                        case RLA: // ROL + AND
                        {
                              uint8_t old_c = F_CHK(cpu, C);
                              (val & 0x80) ? F_SET(cpu, C) : F_CLR(cpu, C);
                              val = (val << 1) | old_c;
                              WRITE(addr, val);
                              cpu->A &= val;
                              (cpu->A == 0) ? F_SET(cpu, Z) : F_CLR(cpu, Z);
                              (cpu->A & N) ? F_SET(cpu, N) : F_CLR(cpu, N);
                              break;
                        }
                        case SRE: // LSR + EOR
                              (val & 0x01) ? F_SET(cpu, C) : F_CLR(cpu, C);
                              val >>= 1;
                              WRITE(addr, val);
                              cpu->A ^= val;
                              (cpu->A == 0) ? F_SET(cpu, Z) : F_CLR(cpu, Z);
                              (cpu->A & N) ? F_SET(cpu, N) : F_CLR(cpu, N);
                              break;
                        case RRA: // ROR + ADC
                        {
                              uint8_t old_c = F_CHK(cpu, C);
                              (val & 0x01) ? F_SET(cpu, C) : F_CLR(cpu, C);
                              val = (val >> 1) | (old_c << 7);
                              WRITE(addr, val);
                              uint16_t result = cpu->A + val + F_CHK(cpu, C);
                              F_CLR(cpu, V);
                              if (((cpu->A ^ result) & (val ^ result) & 0x80) != 0) {
                                    F_SET(cpu, V);
                              }
                              (result > 0xFF) ? F_SET(cpu, C) : F_CLR(cpu, C);
                              cpu->A = result & 0xFF;
                              (cpu->A == 0) ? F_SET(cpu, Z) : F_CLR(cpu, Z);
                              (cpu->A & N) ? F_SET(cpu, N) : F_CLR(cpu, N);
                              break;
                        }
                        case DCP: // DEC + CMP
                              val--;
                              WRITE(addr, val);
                              uint16_t result = cpu->A - val;
                              (cpu->A >= val) ? F_SET(cpu, C) : F_CLR(cpu, C);
                              ((result & 0xFF) == 0) ? F_SET(cpu, Z) : F_CLR(cpu, Z);
                              ((result & 0xFF) & N) ? F_SET(cpu, N) : F_CLR(cpu, N);
                              break;
                        case ISB: // INC + SBC
                              val++;
                              WRITE(addr, val);
                              uint16_t result_isb = cpu->A - val - (1 - F_CHK(cpu, C));
                              F_CLR(cpu, V);
                              if (((cpu->A ^ result_isb) & (~val ^ result_isb) & 0x80) != 0) {
                                    F_SET(cpu, V);
                              }
                              (result_isb <= 0xFF) ? F_SET(cpu, C) : F_CLR(cpu, C);
                              cpu->A = result_isb & 0xFF;
                              (cpu->A == 0) ? F_SET(cpu, Z) : F_CLR(cpu, Z);
                              (cpu->A & N) ? F_SET(cpu, N) : F_CLR(cpu, N);
                              break;
                        default: break;
                  }
                  break;
            }
            case LAX: // LDA + LDX
                  if (isc.mode == IMT) {
                        cpu->A = data;
                        cpu->X = data;
                  } else {
                        tmp_val = READ(addr);
                        cpu->A = tmp_val;
                        cpu->X = tmp_val;
                  }
                  (cpu->A == 0) ? F_SET(cpu, Z) : F_CLR(cpu, Z);
                  (cpu->A & N) ? F_SET(cpu, N) : F_CLR(cpu, N);
                  break;
            case SAX: // STA + STX
                  WRITE(addr, cpu->A & cpu->X);
                  break;
            case AXS: // LDA #$00, then SBC #$xx.  (a&x) - value
                  if (isc.mode == IMT) {
                        uint16_t result = (cpu->A & cpu->X) - data;
                        (result <= 0xFF) ? F_SET(cpu, C) : F_CLR(cpu, C);
                        cpu->X = result & 0xFF;
                        (cpu->X == 0) ? F_SET(cpu, Z) : F_CLR(cpu, Z);
                        (cpu->X & N) ? F_SET(cpu, N) : F_CLR(cpu, N);
                  }
                  break;
            case ALR: // AND + LSR
                  cpu->A = READ(addr) & cpu->A;
                  (cpu->A & 0x01) ? F_SET(cpu, C) : F_CLR(cpu, C);
                  cpu->A >>= 1;
                  (cpu->A == 0) ? F_SET(cpu, Z) : F_CLR(cpu, Z);
                  F_CLR(cpu, N);
                  break;
            case ARR: // AND + ROR
                  cpu->A = READ(addr) & cpu->A;
                  uint8_t old_c = F_CHK(cpu, C);
                  (cpu->A & 0x01) ? F_SET(cpu, C) : F_CLR(cpu, C);
                  cpu->A = (cpu->A >> 1) | (old_c << 7);
                  (cpu->A == 0) ? F_SET(cpu, Z) : F_CLR(cpu, Z);
                  (cpu->A & N) ? F_SET(cpu, N) : F_CLR(cpu, N);
                  if (F_CHK(cpu, D)) {
                        // decimal mode behavior, complex and may not be needed for basic emu
                  }
                  break;
            case ANC: // AND + Set Carry
                  cpu->A &= READ(addr);
                  (cpu->A & N) ? F_SET(cpu, C) : F_CLR(cpu, C);
                  (cpu->A == 0) ? F_SET(cpu, Z) : F_CLR(cpu, Z);
                  (cpu->A & N) ? F_SET(cpu, N) : F_CLR(cpu, N);
                  break;
            case LAS: // LDA + TSX + value_from_mem
                  tmp_val = READ(addr);
                  cpu->SP = tmp_val;
                  cpu->A = tmp_val;
                  cpu->X = tmp_val;
                  (cpu->A == 0) ? F_SET(cpu, Z) : F_CLR(cpu, Z);
                  (cpu->A & N) ? F_SET(cpu, N) : F_CLR(cpu, N);
                  break;
            case SHY:
            case SHX:
            case SKB:
            case IGN:
            default:
                  break;
      }
      
      // update registers
      cpu->PC  += pc_increment;
      cpu->CLC += cyc_lookup[opcode] + page_cross_penalty;
}

void exec_65o2(MOS65o2 *cpu, int clc_to_run) {
      int start_clc = cpu -> CLC;
      while (cpu -> CLC - start_clc < clc_to_run) {
            if (cpu -> NMI) {
                  itrp_65o2(cpu, ITRP_NMI);
                  cpu -> NMI = false;
            } else if (cpu -> IRQ) {
                  itrp_65o2(cpu, ITRP_IRQ);
                  cpu -> IRQ = false;
            }
            cccec(cpu);
      }
}




/* MARK: TEST DEMO */
#define BOLD_GREEN   "\x1b[32;1m"
#define BOLD_RED     "\x1b[31;1m"
#define RESET        "\x1b[0m"

// helper functions to print CPU state for debugging
void print_cpu_state(MOS65o2 *cpu, const char* label) {
      printf("--- %s ---\n", label);
      printf("A: 0x%02X, X: 0x%02X, Y: 0x%02X, SP: 0x%02X, PC: 0x%04X, CYC: %u\n",
             cpu->A, cpu->X, cpu->Y, cpu->SP, cpu->PC, cpu->CLC);
      printf("Flags: N:%d, V:%d, U:%d, B:%d, D:%d, I:%d, Z:%d, C:%d\n",
             F_CHK(cpu, N) > 0, F_CHK(cpu, V) > 0, F_CHK(cpu, U) > 0, F_CHK(cpu, B) > 0,
             F_CHK(cpu, D) > 0, F_CHK(cpu, I) > 0, F_CHK(cpu, Z) > 0, F_CHK(cpu, C) > 0);
      printf("-----------------\n");
}

int pass_count = 0;
int fail_count = 0;

// Assertion macro
#define TEST_ASSERT(condition, message) \
if (!(condition)) { \
printf("         -> Assertion failed: %s\n", message); \
return false; \
}


#define RUN_TEST(test_func, test_name) \
result = test_func(); \
if (result) { \
printf("    ┃ " BOLD_GREEN "  - %-25s  [PASS]  " RESET "   ┃\n", test_name); \
pass_count++; \
} else { \
printf("    ┃ " BOLD_RED "  - %-25s  [FAIL]  " RESET "   ┃\n", test_name); \
fail_count++; \
}

// Test: Load Instructions (LDA, LDX, LDY)
bool test_load_instructions(void) {
      MOS65o2 cpu;
      // LDA Immediate
      init_65o2(&cpu);
      MEMORY[0x0000] = 0xA9; MEMORY[0x0001] = 0xCD;
      cpu.PC = 0x0000;
      exec_65o2(&cpu, 2);
      TEST_ASSERT(cpu.A == 0xCD && F_CHK(&cpu, Z) == 0 && F_CHK(&cpu, N) != 0, "LDA Immediate failed");
      // LDX Zero Page
      init_65o2(&cpu);
      MEMORY[0x0000] = 0xA6; MEMORY[0x0001] = 0x10; MEMORY[0x0010] = 0x55;
      cpu.PC = 0x0000;
      exec_65o2(&cpu, 3);
      TEST_ASSERT(cpu.X == 0x55 && F_CHK(&cpu, Z) == 0 && F_CHK(&cpu, N) == 0, "LDX Zero Page failed");
      // LDY Absolute, test Z flag
      init_65o2(&cpu);
      MEMORY[0x0000] = 0xAC; MEMORY[0x0001] = 0x00; MEMORY[0x0002] = 0x20; MEMORY[0x2000] = 0x00;
      cpu.PC = 0x0000;
      exec_65o2(&cpu, 4);
      TEST_ASSERT(cpu.Y == 0x00 && F_CHK(&cpu, Z) != 0, "LDY Absolute Z flag failed");
      // LDA Absolute,X with page cross
      init_65o2(&cpu);
      MEMORY[0x0000] = 0xBD; MEMORY[0x0001] = 0xFF; MEMORY[0x0002] = 0x10; MEMORY[0x1100] = 0x12;
      cpu.X = 0x01;
      cpu.PC = 0x0000;
      exec_65o2(&cpu, 5); // 4 + 1 for page cross
      TEST_ASSERT(cpu.A == 0x12, "LDA ABS,X page cross failed");
      return true;
}

// Test: Store Instructions (STA, STX, STY)
bool test_store_instructions(void) {
      MOS65o2 cpu;
      // STA Zero Page
      init_65o2(&cpu);
      cpu.A = 0xDE;
      MEMORY[0x0000] = 0x85; MEMORY[0x0001] = 0x30;
      cpu.PC = 0x0000;
      exec_65o2(&cpu, 3);
      TEST_ASSERT(READ(0x0030) == 0xDE, "STA Zero Page failed");
      // STX Absolute
      init_65o2(&cpu);
      cpu.X = 0xEF;
      MEMORY[0x0000] = 0x8E; MEMORY[0x0001] = 0x00; MEMORY[0x0002] = 0x40;
      cpu.PC = 0x0000;
      exec_65o2(&cpu, 4);
      TEST_ASSERT(READ(0x4000) == 0xEF, "STX Absolute failed");
      return true;
}

// Test: Transfer Instructions (TAX, TXA, TAY, TYA, TSX, TXS)
bool test_transfer_instructions(void) {
      MOS65o2 cpu;
      // TAX
      init_65o2(&cpu);
      cpu.A = 0x5A;
      MEMORY[0x0000] = 0xAA;
      cpu.PC = 0x0000;
      exec_65o2(&cpu, 2);
      TEST_ASSERT(cpu.X == 0x5A, "TAX failed");
      // TXS
      init_65o2(&cpu);
      cpu.X = 0xAF;
      MEMORY[0x0000] = 0x9A;
      cpu.PC = 0x0000;
      exec_65o2(&cpu, 2);
      TEST_ASSERT(cpu.SP == 0xAF, "TXS failed");
      return true;
}

// Test: Stack Instructions (PHA, PHP, PLA, PLP)
bool test_stack_instructions(void) {
      MOS65o2 cpu;
      // PHA & PLA
      init_65o2(&cpu);
      cpu.A = 0x12;
      MEMORY[0x0000] = 0x48; // PHA
      MEMORY[0x0001] = 0x68; // PLA
      cpu.PC = 0x0000;
      exec_65o2(&cpu, 3); // PHA
      TEST_ASSERT(READ(0x01FF) == 0x12 && cpu.SP == 0xFE, "PHA failed");
      exec_65o2(&cpu, 4); // PLA
      TEST_ASSERT(cpu.A == 0x12 && cpu.SP == 0xFF, "PLA failed");
      return true;
}

// Test: Flag Instructions (CLC, SEC, CLV, etc.)
bool test_flag_instructions(void) {
      MOS65o2 cpu;
      // CLC
      init_65o2(&cpu);
      F_SET(&cpu, C);
      MEMORY[0x0000] = 0x18;
      cpu.PC = 0x0000;
      exec_65o2(&cpu, 2);
      TEST_ASSERT(F_CHK(&cpu, C) == 0, "CLC failed");
      // SED
      init_65o2(&cpu);
      F_CLR(&cpu, D);
      MEMORY[0x0000] = 0xF8;
      cpu.PC = 0x0000;
      exec_65o2(&cpu, 2);
      TEST_ASSERT(F_CHK(&cpu, D) != 0, "SED failed");
      return true;
}

// Test: Arithmetic and Logical (ADC, SBC, AND, EOR, ORA, CMP, CPX, CPY)
bool test_arithmetic_logical(void) {
      MOS65o2 cpu;
      // ADC with V flag set
      init_65o2(&cpu);
      cpu.A = 0x50;
      F_CLR(&cpu, C);
      MEMORY[0x0000] = 0x69; MEMORY[0x0001] = 0x50;
      cpu.PC = 0x0000;
      exec_65o2(&cpu, 2);
      TEST_ASSERT(cpu.A == 0xA0 && F_CHK(&cpu, V) != 0, "ADC Overflow failed");
      // SBC
      init_65o2(&cpu);
      cpu.A = 0x01;
      F_SET(&cpu, C); // 0x01 - 0x01 - (1-1) = 0x00
      MEMORY[0x0000] = 0xE9; MEMORY[0x0001] = 0x01;
      cpu.PC = 0x0000;
      exec_65o2(&cpu, 2);
      TEST_ASSERT(cpu.A == 0x00 && F_CHK(&cpu, Z) != 0, "SBC failed");
      // CMP
      init_65o2(&cpu);
      cpu.A = 0x10;
      MEMORY[0x0000] = 0xC9; MEMORY[0x0001] = 0x10;
      cpu.PC = 0x0000;
      exec_65o2(&cpu, 2);
      TEST_ASSERT(F_CHK(&cpu, Z) != 0 && F_CHK(&cpu, C) != 0, "CMP failed");
      return true;
}

// Test: Increment/Decrement (INC, DEC, INX, INY, DEX, DEY)
bool test_inc_dec(void) {
      MOS65o2 cpu;
      // INC Zero Page
      init_65o2(&cpu);
      MEMORY[0x0000] = 0xE6; MEMORY[0x0001] = 0x50; MEMORY[0x0050] = 0x0F;
      cpu.PC = 0x0000;
      exec_65o2(&cpu, 5);
      TEST_ASSERT(READ(0x0050) == 0x10, "INC Zero Page failed");
      // DEX
      init_65o2(&cpu);
      cpu.X = 0x01;
      MEMORY[0x0000] = 0xCA;
      cpu.PC = 0x0000;
      exec_65o2(&cpu, 2);
      TEST_ASSERT(cpu.X == 0x00 && F_CHK(&cpu, Z) != 0, "DEX failed");
      return true;
}

// Test: Shift/Rotate (ASL, LSR, ROL, ROR)
bool test_shift_rotate(void) {
      MOS65o2 cpu;
      // ASL Accumulator, sets C
      init_65o2(&cpu);
      cpu.A = 0xC0;
      MEMORY[0x0000] = 0x0A;
      cpu.PC = 0x0000;
      exec_65o2(&cpu, 2);
      TEST_ASSERT(cpu.A == 0x80 && F_CHK(&cpu, C) != 0, "ASL Accumulator failed");
      // LSR Zero Page
      init_65o2(&cpu);
      MEMORY[0x0000] = 0x46; MEMORY[0x0001] = 0x50; MEMORY[0x0050] = 0x01;
      cpu.PC = 0x0000;
      exec_65o2(&cpu, 5);
      TEST_ASSERT(READ(0x0050) == 0x00 && F_CHK(&cpu, Z) != 0 && F_CHK(&cpu, C) != 0, "LSR Zero Page failed");
      return true;
}

// Test: Jump and Subroutine (JMP, JSR, RTS)
bool test_jmp_jsr_rts(void) {
      MOS65o2 cpu;
      // JMP Absolute
      init_65o2(&cpu);
      MEMORY[0x0000] = 0x4C; MEMORY[0x0001] = 0x00; MEMORY[0x0002] = 0x80;
      cpu.PC = 0x0000;
      exec_65o2(&cpu, 3);
      TEST_ASSERT(cpu.PC == 0x8000, "JMP Absolute failed");
      
      // JSR & RTS
      init_65o2(&cpu);
      MEMORY[0x0000] = 0x20; MEMORY[0x0001] = 0x05; MEMORY[0x0002] = 0x00; // JSR $0005
      MEMORY[0x0005] = 0x60; // RTS
      cpu.PC = 0x0000;
      exec_65o2(&cpu, 6); // JSR
      
      TEST_ASSERT(cpu.PC == 0x0005, "JSR: PC did not jump to subroutine start");
      TEST_ASSERT(READ(0x01FF) == 0x00, "JSR: High byte of return address is wrong");
      TEST_ASSERT(READ(0x01FE) == 0x02, "JSR: Low byte of return address is wrong");
      TEST_ASSERT(cpu.SP == 0xFD, "JSR: Stack pointer is wrong");
      
      exec_65o2(&cpu, 6); // RTS
      TEST_ASSERT(cpu.PC == 0x0003, "RTS failed");
      TEST_ASSERT(cpu.SP == 0xFF, "RTS stack pointer failed");
      
      return true;
}

// Test: Branch Instructions (BCC, BCS, BEQ, BMI, BNE, BPL, BVC, BVS)
bool test_branch_instructions(void) {
      MOS65o2 cpu;
      // BNE (no branch)
      init_65o2(&cpu);
      F_SET(&cpu, Z);
      MEMORY[0x0000] = 0xD0; MEMORY[0x0001] = 0x05;
      cpu.PC = 0x0000;
      exec_65o2(&cpu, 2);
      TEST_ASSERT(cpu.PC == 0x0002, "BNE (no branch) failed");
      // BNE (branch)
      init_65o2(&cpu);
      F_CLR(&cpu, Z);
      MEMORY[0x0000] = 0xD0; MEMORY[0x0001] = 0x05;
      cpu.PC = 0x0000;
      exec_65o2(&cpu, 3);
      TEST_ASSERT(cpu.PC == 0x0007, "BNE (branch) failed");
      return true;
}

// Test: BRK and RTI
bool test_brk_rti(void) {
      MOS65o2 cpu;
      init_65o2(&cpu);
      // BRK
      MEMORY[0xFFFE] = 0x50; MEMORY[0xFFFF] = 0x00; // IRQ/BRK vector
      cpu.PC = 0x1000;
      MEMORY[0x1000] = 0x00; // BRK
      F_CLR(&cpu, I);
      exec_65o2(&cpu, 7);
      TEST_ASSERT(cpu.PC == 0x0050 && F_CHK(&cpu, I) != 0 && cpu.SP == 0xFC, "BRK failed");
      // RTI
      cpu.PC = 0x0050;
      MEMORY[0x0050] = 0x40; // RTI
      // Set up stack for RTI (P, PC_L, PC_H)
      WRITE(0x01FD, cpu.P | B);
      WRITE(0x01FE, (0x1002 & 0xFF));
      WRITE(0x01FF, (0x1002 >> 8));
      cpu.SP = 0xFC;
      exec_65o2(&cpu, 6);
      TEST_ASSERT(cpu.PC == 0x1002 && F_CHK(&cpu, B) != 0 && F_CHK(&cpu, U) != 0, "RTI failed");
      return true;
}

int main(void) {
      printf("\n\n\n\n");
      printf("    ┏━━━ \033[1m6502 Emulator Test Report\033[0m ━━━━━━━━━━━━━┓\n");
      printf("    ┃                                           ┃\n");
      
      bool result;
      
      
      RUN_TEST(test_load_instructions, "Load Instructions......");
      RUN_TEST(test_store_instructions, "Store Instructions.....");
      RUN_TEST(test_transfer_instructions, "Transfer Instructions..");
      RUN_TEST(test_stack_instructions, "Stack Instructions.....");
      RUN_TEST(test_flag_instructions, "Flag Instructions......");
      RUN_TEST(test_arithmetic_logical, "Arithmetic/Logical.....");
      RUN_TEST(test_inc_dec, "Increment/Decrement....");
      RUN_TEST(test_shift_rotate, "Shift/Rotate...........");
      RUN_TEST(test_jmp_jsr_rts, "JMP/JSR/RTS............");
      RUN_TEST(test_branch_instructions, "Branch Instructions....");
      RUN_TEST(test_brk_rti, "BRK/RTI................");
      printf("    ┃                                           ┃ \n");
      printf("    ┃                       Tests Passed: %d/11 ┃ \n", pass_count);
      printf("    ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\n\n\n\n");
      
}
