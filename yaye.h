/*
 * ZSisco
 * yaye.h
 */

#ifndef YAYE_H_
#define YAYE_H_

#include <stdio.h>
#include <stdlib.h>

#define MAX_MEM 4096

/* Condition flags */
#define ZF 0
#define SF 1
#define OF 2

/* Status condition codes */
#define AOK 1
#define HLT 2
#define ADR 3
#define INS 4

/* Register index of stack pointer */
#define ESP 4

/* Opcodes and their mnemonics */
#define HALT   0x00
#define NOP    0x10
#define RRMOVL 0x20
#define IRMOVL 0x30
#define RMMOVL 0x40
#define MRMOVL 0x50
#define ADDL   0x60
#define SUBL   0x61
#define ANDL   0x62
#define XORL   0x63
#define JMP    0x70
#define JLE    0x71
#define JL     0x72
#define JE     0x73
#define JNE    0x74
#define JGE    0x75
#define JG     0x76
#define CMOVLE 0x21
#define CMOVL  0x22
#define CMOVE  0x23
#define CMOVNE 0x24
#define CMOVGE 0x25
#define CMOVG  0x26
#define CALL   0x80
#define RET    0x90
#define PUSHL  0xa0
#define POPL   0xb0


/* Given 2 ASCII characters, convert them into a single hexadecimal number */
unsigned char convert_ascii_to_hex(char n1, char n2);

/* Load program file, initialize memory */
int load_program(const char *filename);

/* Given 4 1-byte hexadecimal numbers, reorient them into a single 4-byte value */
int convert_word_to_value(unsigned char a, unsigned char b, unsigned char c, unsigned char d);

/* Write word to memory */
int write_word(int word, int dest);

/* Read word from memory */
int read_word(int src);

/* Check that register index is between 0 - 7. Return 0 if valid, 1 if not. */
int valid_register(char r);

/* Check that address is in valid memory. Return 0 if valid, 1 if not. */
int valid_address(long addr);

/* Set zero flag */
void set_ZF(int result);

/* Set sign flag */
void set_SF(int result);

/* Set overflow flag */
void set_OF(int oper1, int oper2, int result, unsigned char opcode);

/* Given a valid register index, return a pointer to the corresponding register */
int *get_reg(unsigned char reg_index);

/* Instruction declarations */
void halt();

void nop();

void rrmovl(unsigned char rA, unsigned char rB);

void irmovl(int value, unsigned char rB);

void rmmovl(unsigned char rA, int displacement, unsigned char rB);

void mrmovl(int displacement, unsigned char rB, unsigned char rA);

void addl(unsigned char rA, unsigned char rB);

void subl(unsigned char rA, unsigned char rB);

void andl(unsigned char rA, unsigned char rB);

void xorl(unsigned char rA, unsigned char rB);

void jmp(int dest);

void jle(int dest);

void jl(int dest);

void je(int dest);

void jne(int dest);

void jge(int dest);

void jg(int dest);

void cmovle(unsigned char rA, unsigned char rB);

void cmovl(unsigned char rA, unsigned char rB);

void cmove(unsigned char rA, unsigned char rB);

void cmovne(unsigned char rA, unsigned char rB);

void cmovge(unsigned char rA, unsigned char rB);

void cmovg(unsigned char rA, unsigned char rB);

void pushl(unsigned char rA);

void popl(unsigned char rA);

void call(int dest);

void ret();

/* Print out register values, condition flags, and program counter. Display status errors if any. */
void post_execution_print();

void emulate();

#endif
