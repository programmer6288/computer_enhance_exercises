#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <iostream>
#define INST_BUF_SIZE 8
#define R1 0b00111000
#define R2 0b00000111
typedef unsigned int uint;

char *reg_arr[] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh", "ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
int main(int argc, char *argv[]) {
    char inst_buf[INST_BUF_SIZE];
    char next_inst[50];
    FILE *fp = fopen(argv[1], "rb");
    FILE *asm_file = fopen(argv[2], "wb");
    size_t num_bytes;
    do {
        num_bytes = fread(inst_buf, sizeof(*inst_buf), INST_BUF_SIZE, fp);
        for (int i = 0; i < num_bytes; i+=2) {
            char d = inst_buf[i] & 2;
            char w = inst_buf[i] & 1;
            char r1, r2;
            if (d) {
                r1 = (inst_buf[i + 1] & R1) >> 3;
                r2 = inst_buf[i + 1] & R2; 
            } else {
                r2 = (inst_buf[i + 1] & R1) >> 3;
                r1 = inst_buf[i + 1] & R2; 
            }
            uint inst_ptr = 0;
            memcpy(&next_inst[inst_ptr], "mov ", 4);
            inst_ptr += 4;
            memcpy(&next_inst[inst_ptr], reg_arr[w << 3 | r1], 2);
            inst_ptr += 2;
            memcpy(&next_inst[inst_ptr], ", ", 2);
            inst_ptr += 2;
            memcpy(&next_inst[inst_ptr], reg_arr[w << 3 | r2], 2);
            inst_ptr += 2;
            memcpy(&next_inst[inst_ptr], "\n", 1);
            inst_ptr += 1;        
            fwrite(next_inst, sizeof(*next_inst), inst_ptr, asm_file);
        }
    } while (num_bytes > 0);
    return 0;
}