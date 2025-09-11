#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define DATA_SIZE 30000

uint8_t data[DATA_SIZE] = {0};

// cmd enum
typedef enum {
    ICC_CMD, // > increment cursor
    DEC_CMD, // < decrement cursor
    ICV_CMD, // + increment value
    DEV_CMD, // - decrement value
    OUT_CMD, // . output (as ASCII)
    INP_CMD, // , input
    LOS_CMD, // [ loop start
    LOE_CMD, // ] loop end
    UNK_CMD  // unknown command
} CMD;

// cmd func
static void ICC(uint8_t **memory);
static void DEC(uint8_t **memory);
static void ICV(uint8_t **memory);
static void DEV(uint8_t **memory);
static void OUT(uint8_t **memory);
static void INP(uint8_t **memory);
bool is_legal(const char *program);
void exec_program(const char *program);

typedef void (*EXEC)(uint8_t **memory);

bool is_legal(const char *program) {
      size_t len = strlen(program);
      int balance = 0;

      for (size_t i = 0; i < len; i++) {
            if (program[i] == '[') {
                  balance++;
            } else if (program[i] == ']') {
                  balance--;
            }
            if (balance < 0) {
                  return false;
            }
      }
      return balance == 0;
}

void ICC(uint8_t **memory) {
    if (*memory < &data[DATA_SIZE - 1]) {
        (*memory)++;
    }
}

void DEC(uint8_t **memory) {
    if (*memory > &data[0]) {
        (*memory)--;
    }
}

void ICV(uint8_t **memory) { (**memory)++; }
void DEV(uint8_t **memory) { (**memory)--; }
void OUT(uint8_t **memory) { putchar(**memory); }
void INP(uint8_t **memory) { **memory = getchar(); }

// cmd to enum
static CMD to_cmd(char c) {
    switch (c) {
        case '>': return ICC_CMD;
        case '<': return DEC_CMD;
        case '+': return ICV_CMD;
        case '-': return DEV_CMD;
        case '.': return OUT_CMD;
        case ',': return INP_CMD;
        case '[': return LOS_CMD;
        case ']': return LOE_CMD;
    }
    return UNK_CMD;
}

// cmd ptr array
EXEC cmds[] = {
    ICC,
    DEC,
    ICV,
    DEV,
    OUT,
    INP,
    NULL, // LOS_CMD
    NULL, // LOE_CMD
    NULL  // UNK_CMD
};

void exec_program(const char *program) {
    uint8_t *memory_ptr = data;
    size_t len = strlen(program);
    size_t pc = 0;

    while (pc < len) {
        char current_char = program[pc];

        if (current_char == '[') {
            if (*memory_ptr == 0) {
                int balance = 1;
                while (balance != 0) {
                    pc++;
                    if (pc >= len) {break; } 
                    if (program[pc] == '[') balance++;
                    else if (program[pc] == ']') balance--;
                }
            }
        } else if (current_char == ']') {
            if (*memory_ptr != 0) {
                int balance = -1;
                while (balance != 0) {
                    pc--;
                    if (pc < 0) {break; }
                    if (program[pc] == ']') balance--;
                    else if (program[pc] == '[') balance++;
                }
            }
        } else {
            CMD cmd = to_cmd(current_char);
            if (cmd != UNK_CMD) {
                cmds[cmd](&memory_ptr);
            }
        }
        pc++;
    }
}



int main(void) {
    const char *program = "++++++++[>++++[>++>+++>+++>+<<<<<-]>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++.";    if (!is_legal(program)) {
        printf("Program Wrong...");
        return 0;
    }

    exec_program(program);
    putchar('\n');

    return 0;
}