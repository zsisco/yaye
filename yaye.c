/*
 * ZSisco
 * yaye.c
 */

#include "yaye.h"

/* The array of program memory */
unsigned char memory[MAX_MEM] = { 0 };

/* Initialize the registers */
int reg[8] = { 0 };

/* Initialize the condition flags */
unsigned char flags[3] = { 0 };

/* Initialize the program counter */
unsigned long pc = 0;

/* Initialize the program status */
char status = AOK;

unsigned long program_size = 0;


unsigned char convert_ascii_to_hex(char n1, char n2) {

    /* Convert n1 */
    /* If '0' - '9' */
    if (n1 > '/' && n1 < ':') {
        n1 = n1 - 48;
    }
    /* If 'a' - 'f' */
    else if (n1 > '`' && n1 < 'g') {
        n1 = n1 - 87;
    }

    /* Convert n2 */
    /* If '0' - '9' */
    if (n2 > '/' && n2 < ':') {
        n2 = n2 - 48;
    }
    /* If 'a' - 'f' */
    else if (n2 > '`' && n2 < 'g') {
        n2 = n2 - 87;
    }

    /* Combine n1 and n2 */
    return ( (n1 << 4) & 0xf0) | n2;
}


int load_program(const char *filename) {

    /* Boiler plate file reading code */
    FILE *file = fopen(filename, "rt");
    char *buffer = 0;
    unsigned long buffer_size = 0;

    if (file == NULL) {
        fprintf(stderr, "\nLOAD PROGRAM ERROR: File '%s' not valid.\n", filename);
        return 1;
    }
    
    fseek(file, 0, SEEK_END);
    buffer_size = ftell(file);
    
    if (buffer_size > MAX_MEM) {
        fprintf(stderr, "\nLOAD PROGRAM ERROR: File size cannot be greater than %lu bytes.\n", (unsigned long)MAX_MEM);
        return 1;
    }

    rewind(file);

    buffer = malloc(buffer_size);
    fread(buffer, 1, buffer_size, file);
    fclose(file);

    program_size = MAX_MEM;

    for (int i = 0; i < buffer_size; i++) {
        /* Load converted hex. values in memory array */
        memory[i] = convert_ascii_to_hex(buffer[2*i], buffer[2*i + 1]);
    }

    free(buffer);
    buffer = NULL;

    return 0;
}


int convert_word_to_value(unsigned char a, unsigned char b, unsigned char c, unsigned char d) {

    int value = 0;

    value = b << 8 | a;
    value = c << 16 | value;
    value = d << 24 | value;

    return value;
}


int write_word(int word, int dest) {

    if (valid_address(dest) == 1 || valid_address(dest + 3) == 1) {
        return 1;
    }

    memory[dest] = word & 0x000000ff;
    memory[dest + 1] = (word & 0x0000ff00) >> 8;
    memory[dest + 2] = (word & 0x00ff0000) >> 16;
    memory[dest + 3] = (word & 0xff000000) >> 24;
    
    return 0;
}


int read_word(int src) {

    return convert_word_to_value(memory[src], memory[src + 1], memory[src + 2], memory[src + 3]);
}


/* Check that register index is between 0 - 7 */
int valid_register(char r) {

    return ( r < 8 && r >= 0 ) ? 0 : 1;
}


/* Check that address is in valid memory */
int valid_address(long addr) {

    return ( addr < program_size && addr >= 0 ) ? 0 : 1;
}

void set_ZF(int result) {

    flags[ZF] = result == 0 ? 1 : 0;
}

void set_SF(int result) {

    flags[SF] = result < 0 ? 1 : 0;
}

void set_OF(int oper1, int oper2, int result, unsigned char opcode) {

    /* Addition overflow: sign of result is changed by adding two numbers with same sign */
    if (opcode == ADDL && (oper1 > 0 && oper2 > 0 && result < 0) ) {
        flags[OF] = 1;
    }
    else if (opcode == ADDL && (oper1 < 0 && oper2 < 0 && result > 0) ) {
        flags[OF] = 1;
    }
    /* Subtraction overflow: sign of result is changed by subtracting two numbers with opposite signs */
    else if (opcode == SUBL && ( (result < oper2) != (oper1 > 0) )) {
        flags[OF] = 1;
    }
    else {
        flags[OF] = 0;
    }
}


int *get_reg(unsigned char reg_index) {

    return &reg[reg_index];
}


void halt() {
    
    status = HLT;
    printf("\thalt\n\n");
}


void nop() {

    pc += 1;
    printf("\tnop\n\n");
}


void rrmovl(unsigned char rA, unsigned char rB) {

    if (valid_register(rA) == 1 || valid_register(rB) == 1) {
        status = INS;
        return;
    }
    
    int *src = get_reg(rA);
    int *dest = get_reg(rB);

    *dest = *src;

    pc += 2;
    printf("\trrmovl %d, %d\n\n",rA,rB);
}


void irmovl(int value, unsigned char rB) {

    if (valid_register(rB) == 1) {
        status = INS;
        return;
    }

    int *dest = get_reg(rB);
    *dest = value; 

    pc += 6;
    printf("\tirmovl 0x%x, %d\n\n",reg[rB],rB);
}


void rmmovl(unsigned char rA, int displacement, unsigned char rB) {

    if (valid_register(rA) == 1 || valid_register(rB) == 1) {
        status = INS;
        return;
    }

    int *src = get_reg(rA);
    int *dest = get_reg(rB);

    if (write_word(*src, displacement + *dest) == 1) {
        status = ADR;
        return;
    }

    pc += 6;
    printf("\trmmovl %d, 0x%x(%d)\n\n",rA,displacement,rB);
}


void mrmovl(int displacement, unsigned char rB, unsigned char rA) {

    if (valid_register(rA) == 1 || valid_register(rB) == 1) {
        status = INS;
        return;
    }

    int *dest = get_reg(rA);
    int *src = get_reg(rB);

    if (valid_address(displacement + *src) == 1) {
        status = ADR;
        return;
    }

    *dest = read_word(displacement + *src);

    pc += 6;
    printf("\tmrmovl 0x%x(%d), %d\n\n",displacement,rB,rA);
}


void addl(unsigned char rA, unsigned char rB) {

    if (valid_register(rA) == 1 || valid_register(rB) == 1) {
        status = INS;
        return;
    }
    
    int *src = get_reg(rA);
    int *dest = get_reg(rB);
    int result = *src + *dest;

    /* Set condition flags */
    set_ZF(result);
    set_SF(result);
    set_OF(*src, *dest, result, ADDL);

    *dest = result;

    pc += 2;
    printf("\taddl %d, %d\n\n",rA,rB);
}


void subl(unsigned char rA, unsigned char rB) {
    
    if (valid_register(rA) == 1 || valid_register(rB) == 1) {
        status = INS;
        return;
    }
    
    int *src = get_reg(rA);
    int *dest = get_reg(rB);
    int result = *dest - *src;

    /* Set condition flags */
    set_ZF(result);
    set_SF(result);
    set_OF(*src, *dest, result, SUBL);

    *dest = result;

    pc += 2;
    printf("\tsubl %d, %d\n\n",rA,rB);
}


void andl(unsigned char rA, unsigned char rB) {

    if (valid_register(rA) == 1 || valid_register(rB) == 1) {
        status = INS;
        return;
    }
    
    int *src = get_reg(rA);
    int *dest = get_reg(rB);
    int result = *src & *dest;

    /* Set condition flags */
    set_ZF(result);
    set_SF(result);
    set_OF(*src, *dest, result, ANDL);

    *dest = result;

    pc += 2;
    printf("\tandl %d, %d\n\n",rA,rB);
}


void xorl(unsigned char rA, unsigned char rB) {

    if (valid_register(rA) == 1 || valid_register(rB) == 1) {
        status = INS;
        return;
    }
    
    int *src = get_reg(rA);
    int *dest = get_reg(rB);
    int result = *src ^ *dest;

    /* Set condition flags */
    set_ZF(result);
    set_SF(result);
    set_OF(*src, *dest, result, XORL);

    *dest = result;

    pc += 2;
    printf("\txorl %d, %d\n\n",rA,rB);
}


void jmp(int dest) {

    if (valid_address(dest) == 1) {
        status = ADR;
        return;
    }

    pc = dest;
    printf("\tjmp 0x%x\n\n",dest);
}


void jle(int dest) {

    if (valid_address(dest) == 1) {
        status = ADR;
        return;
    }

    if (flags[ZF] == 1 || flags[SF] != flags[OF]) {
        pc = dest;
    }
    else {
        pc += 5;
    }
    printf("\tjle 0x%x\n\n",dest);
}


void jl(int dest) {

    if (valid_address(dest) == 1) {
        status = ADR;
        return;
    }

    if (flags[SF] != flags[OF]) {
        pc = dest;
    }
    else {
        pc += 5;
    }
    printf("\tjl 0x%x\n\n",dest);
}


void je(int dest) {

    if (valid_address(dest) == 1) {
        status = ADR;
        return;
    }

    if (flags[ZF] == 1) {
        pc = dest;
    }
    else {
        pc += 5;
    }
    printf("\tje 0x%x\n\n",dest);
}


void jne(int dest) {

    if (valid_address(dest) == 1) {
        status = ADR;
        return;
    }

    if (flags[ZF] == 0) {
        pc = dest;
    }
    else {
        pc += 5;
    }
    printf("\tjne 0x%x\n\n",dest);
}


void jge(int dest) {

    if (valid_address(dest) == 1) {
        status = ADR;
        return;
    }

    if (flags[SF] == flags[OF]) {
        pc = dest;
    }
    else {
        pc += 5;
    }
    printf("\tjge 0x%x\n\n",dest);
}


void jg(int dest) {

    if (valid_address(dest) == 1) {
        status = ADR;
        return;
    }

    if (flags[ZF] == 0 && flags[SF] == flags[OF]) {
        pc = dest;
    }
    else {
        pc += 5;
    }
    printf("\tjg 0x%x\n\n",dest);
}


void cmovle(unsigned char rA, unsigned char rB) {

    if (flags[ZF] == 1 || flags[SF] != flags[OF]) {
        rrmovl(rA, rB);
        printf("\t-> called by cmovle %d, %d\n\n",rA,rB);
    }
    else {
        pc += 2;
    }
}


void cmovl(unsigned char rA, unsigned char rB) {

    if (flags[SF] != flags[OF]) {
        rrmovl(rA, rB);
        printf("\t-> called by cmovl %d, %d\n\n",rA,rB);
    }
    else {
        pc += 2;
    }
}


void cmove(unsigned char rA, unsigned char rB) {

    if (flags[ZF] == 1) {
        rrmovl(rA, rB);
        printf("\t-> called by cmove %d, %d\n\n",rA,rB);
    }
    else {
        pc += 2;
    }
}


void cmovne(unsigned char rA, unsigned char rB) {

    if (flags[ZF] == 0) {
        rrmovl(rA, rB);
        printf("\t-> called by cmovne %d, %d\n\n",rA,rB);
    }
    else {
        pc += 2;
    }
}


void cmovge(unsigned char rA, unsigned char rB) {

    if (flags[SF] == flags[OF]) {
        rrmovl(rA, rB);
        printf("\t-> called by cmovge %d, %d\n\n",rA,rB);
    }
    else {
        pc += 2;
    }
}


void cmovg(unsigned char rA, unsigned char rB) {

    if (flags[ZF] == 0 && flags[SF] == flags[OF]) {
        rrmovl(rA, rB);
        printf("\t-> called by cmovg %d, %d\n\n",rA,rB);
    }
    else {
        pc += 2;
    }
}


void pushl(unsigned char rA) {

    if (valid_register(rA) == 1) {
        status = INS;
        return;
    }

    int *esp = get_reg(ESP);
    int *src = get_reg(rA);

    *esp = *esp - 4;

    if (write_word(*src, *esp) == 1) {
        status = ADR;
        return;
    }

    pc += 2;
    printf("\tpushl %d\n\n",rA);
}


void popl(unsigned char rA) {

    if (valid_register(rA) == 1) {
        status = INS;
        return;
    }

    int *dest = get_reg(rA);
    int *esp = get_reg(ESP);

    if (valid_address(*esp) == 1) {
        status = ADR;
        return;
    }

    *dest = read_word(*esp);
    *esp = *esp + 4;

    pc += 2;
    printf("\tpopl %d\n\n",rA);
}


void call(int dest) {

    if (valid_address(dest) == 1) {
        status = ADR;
        return;
    }

    int *esp = get_reg(ESP);

    *esp = *esp - 4;

    if (write_word(pc + 5, *esp) == 1) {
        status = ADR;
        return;
    }

    pc = dest;
    printf("\tcall 0x%x\n\n",dest);
}


void ret() {

    int *esp = get_reg(ESP);

    pc = read_word(*esp);

    *esp = *esp + 4;
    printf("\tret pc 0x%lx (%lu)\n\n",pc,pc);
}


void post_execution_print() {

    printf("\n******************************************************\n");

    /* If status is not AOK or HLT */
    if (status == ADR) {
        fprintf(stderr, "ADR error: Invalid address encountered.\n\n");
    }
    else if (status == INS) {
        fprintf(stderr, "INS error: Invalid instruction encountered.\n\n");
    }

    /* Print registers, program counter, condition flags */
    printf("Registers:\n");
    printf("\teax: 0x%08x (%d)\n",reg[0],reg[0]);
    printf("\tecx: 0x%08x (%d)\n",reg[1],reg[1]);
    printf("\tedx: 0x%08x (%d)\n",reg[2],reg[2]);
    printf("\tebx: 0x%08x (%d)\n",reg[3],reg[3]);
    printf("\tesp: 0x%08x (%d)\n",reg[4],reg[4]);
    printf("\tebp: 0x%08x (%d)\n",reg[5],reg[5]);
    printf("\tesi: 0x%08x (%d)\n",reg[6],reg[6]);
    printf("\tedi: 0x%08x (%d)\n",reg[7],reg[7]);
    printf("\nFlags: ZF %hhu, SF %hhu, OF %hhu\n\n",flags[ZF],flags[SF],flags[OF]);
    printf("Program Counter: 0x%lx (%lu)\n\n",pc,pc);
}


void emulate() {

    /* This variable is used for any of the instructions that require a 4-byte word as an operand */
    int word = 0;

    while (1) {

        if (valid_address(pc) == 1) {
            status = ADR;
        }

        if (status != AOK) {
            break;
        }

        switch (memory[pc]) {

            case HALT:
                halt();
                break;

            case NOP:
                nop();
                break;

            case RRMOVL:
                rrmovl((memory[pc + 1] & 0xf0) >> 4, memory[pc + 1] & 0x0f);
                break;

            case IRMOVL:
                word = convert_word_to_value(memory[pc + 2], memory[pc + 3], memory[pc + 4], memory[pc + 5]);
                irmovl(word, memory[pc + 1] & 0x0f);
                break;

            case RMMOVL:
                word = convert_word_to_value(memory[pc + 2], memory[pc + 3], memory[pc + 4], memory[pc + 5]);
                rmmovl((memory[pc + 1] & 0xf0) >> 4, word, memory[pc + 1] & 0x0f);
                break;

            case MRMOVL:
                word = convert_word_to_value(memory[pc + 2], memory[pc + 3], memory[pc + 4], memory[pc + 5]);
                mrmovl(word, memory[pc + 1] & 0x0f, (memory[pc + 1] & 0xf0) >> 4);
                break;

            case ADDL:
                addl((memory[pc + 1] & 0xf0) >> 4, memory[pc + 1] & 0x0f);
                break;

            case SUBL:
                subl((memory[pc + 1] & 0xf0) >> 4, memory[pc + 1] & 0x0f);
                break;

            case ANDL:
                andl((memory[pc + 1] & 0xf0) >> 4, memory[pc + 1] & 0x0f);
                break;

            case XORL:
                xorl((memory[pc + 1] & 0xf0) >> 4, memory[pc + 1] & 0x0f);
                break;

            case JMP:
                word = convert_word_to_value(memory[pc + 1], memory[pc + 2], memory[pc + 3], memory[pc + 4]);
                jmp(word);
                break;

            case JLE:
                word = convert_word_to_value(memory[pc + 1], memory[pc + 2], memory[pc + 3], memory[pc + 4]);
                jle(word);
                break;

            case JL:
                word = convert_word_to_value(memory[pc + 1], memory[pc + 2], memory[pc + 3], memory[pc + 4]);
                jl(word);
                break;

            case JE:
                word = convert_word_to_value(memory[pc + 1], memory[pc + 2], memory[pc + 3], memory[pc + 4]);
                je(word);
                break;

            case JNE:
                word = convert_word_to_value(memory[pc + 1], memory[pc + 2], memory[pc + 3], memory[pc + 4]);
                jne(word);
                break;

            case JGE:
                word = convert_word_to_value(memory[pc + 1], memory[pc + 2], memory[pc + 3], memory[pc + 4]);
                jge(word);
                break;

            case JG:
                word = convert_word_to_value(memory[pc + 1], memory[pc + 2], memory[pc + 3], memory[pc + 4]);
                jg(word);
                break;

            case CMOVLE:
                cmovle((memory[pc + 1] & 0xf0) >> 4, memory[pc + 1] & 0x0f);
                break;

            case CMOVL:
                cmovl((memory[pc + 1] & 0xf0) >> 4, memory[pc + 1] & 0x0f);
                break;

            case CMOVE:
                cmove((memory[pc + 1] & 0xf0) >> 4, memory[pc + 1] & 0x0f);
                break;

            case CMOVNE:
                cmovne((memory[pc + 1] & 0xf0) >> 4, memory[pc + 1] & 0x0f);
                break;

            case CMOVGE:
                cmovge((memory[pc + 1] & 0xf0) >> 4, memory[pc + 1] & 0x0f);
                break;

            case CMOVG:
                cmovg((memory[pc + 1] & 0xf0) >> 4, memory[pc + 1] & 0x0f);
                break;

            case CALL:
                word = convert_word_to_value(memory[pc + 1], memory[pc + 2], memory[pc + 3], memory[pc + 4]);
                call(word);
                break;

            case RET:
                ret();
                break;

            case PUSHL:
                pushl((memory[pc + 1] & 0xf0) >> 4);
                break;

            case POPL:
                popl((memory[pc + 1] & 0xf0) >> 4);
                break;

            default:
                fprintf(stderr, "\nUnknown opcode %x\n",memory[pc]);
                status = INS;
        }
        
        /* Reset word variable before next fetch/decode cycle */
        word = 0; 
    }

    post_execution_print();
}


int main(int argc, char **argv) {


    /* Argument validation */
    if (argc < 2) {
        fprintf(stderr, "\nUSAGE ERROR: yaye requires a program file as argument.\n      Usage: ./yaye <filename>\n\n");
        return 1;
    }
    else if (argc > 2) {
        fprintf(stderr, "\nUSAGE ERROR: Too many arguments.\n      Usage: ./yaye <filename>\n\n");
        return 1;
    }

    /* Load program from argument */
    if (load_program(argv[1]) == 1) {
        printf("\nUSAGE ERROR: Cannot load program. Exiting.\n\n");
        return 1;
    }

    emulate();

    return 0;
}
