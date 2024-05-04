#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#define MAX_INST_SIZE 6
const char *reg_arr[] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh", "ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
const char *mem_addresses[] = {
    "bx + si",
    "bx + di",
    "bp + si",
    "bp + di",
    "si",
    "di",
    "bp",
    "bx"
};
int main(int argc, char *argv[]) {
    char inst_buf[MAX_INST_SIZE];
    char next_inst[100];
    size_t inst_ptr = 0;
    size_t inst_size = 0;
    FILE *fp = fopen(argv[1], "rb");
    FILE *asm_file = fopen(argv[2], "wb");
    fwrite("bits 16\n", 1, 8, asm_file);
    bool reg_mem = false;
    bool imm = false;
    bool imm_reg = false;
    bool accum = false;
    bool d;
    bool w;
    while (fread(&inst_buf[inst_size], sizeof(*inst_buf), 1, fp)) {
        inst_size++;
        if (inst_size == 1) {
            if ((inst_buf[0] & 0b11111100) == 0b10100000) {
                accum = true;
                d = inst_buf[0] & 0b00000010;
                w = inst_buf[0] & 1;
            } else if ((inst_buf[0] & 0b11111100) == 0b10001000) {
                reg_mem = true;
                d = inst_buf[0] & 0b00000010;
                w = inst_buf[0] & 1;
            } else if ((inst_buf[0] & 0b11111110) == 0b11000110) {
                imm = true;
                w = inst_buf[0] & 1;
            } else if ((inst_buf[0] & 0b11110000) == 0b10110000) {
                imm_reg = true;
                w = inst_buf[0] & 0b1000;
            }
        } else if (inst_size == 2) {
            if (accum) {
                fread(&inst_buf[inst_size], sizeof(*inst_buf), 1, fp);
                inst_size++;
                unsigned short mem_addr = inst_buf[2] << 8 | (unsigned char) inst_buf[1];
                if (d) {
                    inst_ptr = sprintf(next_inst, "mov [%d], %s\n", mem_addr, w ? "ax" : "al");
                } else {
                    inst_ptr = sprintf(next_inst, "mov %s, [%d]\n", w ? "ax" : "al", mem_addr);
                }
                fwrite(next_inst, sizeof(*next_inst), inst_ptr, asm_file);
                inst_ptr = 0;
                inst_size = 0;
                accum = false;
                d = false;
            } else if (imm_reg) {
                char reg = inst_buf[0] & 0b111;
                if (w) {
                    fread(&inst_buf[inst_size], sizeof(*inst_buf), 1, fp);
                    inst_size++;
                    short imm_val = inst_buf[2] << 8 | (unsigned char) inst_buf[1];
                    inst_ptr = sprintf(next_inst, "mov %s, %d\n", reg_arr[reg + (w << 3)], imm_val);
                } else {
                    char imm_val = inst_buf[1];
                    inst_ptr = sprintf(next_inst, "mov %s, %d\n", reg_arr[reg + (w << 3)], imm_val);
                }
                fwrite(next_inst, sizeof(*next_inst), inst_ptr, asm_file);
                inst_ptr = 0;
                inst_size = 0;
                w = false;
                imm_reg = false;
            } else if (reg_mem) {
                char rm = inst_buf[1] & 0b111;
                char reg = (inst_buf[1] & 0b00111000) >> 3;
                if ((inst_buf[1] & 0b11000000) == 0) {
                    if (rm == 0b110) {
                        // Direct address, instruction ends
                        if (w) {
                            fread(&inst_buf[inst_size], sizeof(*inst_buf), 2, fp);
                            unsigned short disp = inst_buf[3] << 8 | (unsigned char) inst_buf[2];

                            if (d) {
                                inst_ptr = sprintf(next_inst, "mov %s, [%d]\n", reg_arr[reg + (w << 3)], disp);
                            } else {
                                inst_ptr = sprintf(next_inst, "mov [%d], %s\n", disp, reg_arr[reg + (w << 3)]);
                            }
                        } else {
                            fread(&inst_buf[inst_size], sizeof(*inst_buf), 1, fp);
                            unsigned char disp = inst_buf[2];

                            if (d) {
                                inst_ptr = sprintf(next_inst, "mov %s, [%d]\n", reg_arr[reg + (w << 3)], disp);
                            } else {
                                inst_ptr = sprintf(next_inst, "mov [%d], %s\n", disp, reg_arr[reg + (w << 3)]);
                            }
                        } 
                        fwrite(next_inst, sizeof(*next_inst), inst_ptr, asm_file);
                        inst_ptr = 0;
                        inst_size = 0;
                        reg_mem = false;
                        d = false;
                        w = false;
                    } else {
                        // No displacement, instruction ends
                        if (d) {
                            inst_ptr = sprintf(next_inst, "mov %s, [%s]\n", reg_arr[reg + (w << 3)], mem_addresses[rm]);
                        } else {
                            inst_ptr = sprintf(next_inst, "mov [%s], %s\n", mem_addresses[rm], reg_arr[reg + (w << 3)]);
                        }
                        fwrite(next_inst, sizeof(*next_inst), inst_ptr, asm_file);
                        inst_ptr = 0;
                        inst_size = 0;
                        reg_mem = false;
                        d = false;
                        w = false;
                    }
                } else if ((inst_buf[1] & 0b11000000) == 0b01000000) {
                    // 8-bit displacement
                    fread(&inst_buf[inst_size], sizeof(*inst_buf), 1, fp);
                    inst_size++;
                    char disp = inst_buf[2];
                    if (d) {
                        inst_ptr = sprintf(next_inst, "mov %s, [%s + %d]\n", reg_arr[reg + (w << 3)], mem_addresses[rm], disp);
                    } else {
                        inst_ptr = sprintf(next_inst, "mov [%s + %d], %s\n", mem_addresses[rm], disp, reg_arr[reg + (w << 3)]);
                    }

                    fwrite(next_inst, sizeof(*next_inst), inst_ptr, asm_file);
                    inst_ptr = 0;
                    inst_size = 0;
                    reg_mem = false;
                    d = false;
                    w = false;
                } else if ((inst_buf[1] & 0b11000000) == 0b10000000) {
                    // 16-bit displacement
                    fread(&inst_buf[inst_size], sizeof(*inst_buf), 2, fp);
                    inst_size += 2;
                    short disp = inst_buf[3] << 8 | (unsigned char) inst_buf[2];
                    if (d) {
                        inst_ptr = sprintf(next_inst, "mov %s, [%s + %d]\n", reg_arr[reg + (w << 3)], mem_addresses[rm], disp);
                    } else {
                        inst_ptr = sprintf(next_inst, "mov [%s + %d], %s\n", mem_addresses[rm], disp, reg_arr[reg + (w << 3)]);
                    }
                    
                    fwrite(next_inst, sizeof(*next_inst), inst_ptr, asm_file);
                    inst_ptr = 0;
                    inst_size = 0;
                    reg_mem = false;
                    d = false;
                    w = false;
                } else {
                    // Reg to reg, instruction ends
                    char r1, r2;
                    if (d) {
                        r1 = reg;
                        r2 = rm;
                    } else {
                        r1 = rm;
                        r2 = reg;
                    }
                    inst_ptr = sprintf(next_inst, "mov %s, %s\n", reg_arr[r1 + (w << 3)], reg_arr[r2 + (w << 3)]);
                    fwrite(next_inst, sizeof(*next_inst), inst_ptr, asm_file);
                    inst_ptr = 0;
                    inst_size = 0;
                    d = false;
                    w = false;
                }
            } else if (imm) {
                char rm = inst_buf[1] & 0b111;
                // Immediate to reg/mem
                if ((inst_buf[1] & 0b11000000) == 0b11000000) {
                    // Immediate to register
                    if (w) {
                        fread(&inst_buf[inst_size], sizeof(*inst_buf), 2, fp);
                        inst_size += 2;
                        short imm_val = inst_buf[3] << 8 | (unsigned char) inst_buf[2];
                        inst_ptr = sprintf(next_inst, "mov %s, %d\n", reg_arr[rm + (w << 3)], imm_val);
                    } else {
                        fread(&inst_buf[inst_size], sizeof(*inst_buf), 1, fp);
                        inst_size++;
                        char imm_val = inst_buf[2];
                        inst_ptr = sprintf(next_inst, "mov %s, %d\n", reg_arr[rm + (w << 3)], imm_val);
                    }
                    fwrite(next_inst, sizeof(*next_inst), inst_ptr, asm_file);
                    inst_ptr = 0;
                    inst_size = 0;
                    w = false;
                    imm = false;
                } else if ((inst_buf[1] & 0b11000000) == 0b10000000) {
                    // Immediate to memory with 16-bit displacement
                    if (w) {
                        fread(&inst_buf[inst_size], sizeof(*inst_buf), 4, fp);
                        unsigned short disp = inst_buf[3] << 8 | (unsigned char) inst_buf[2];
                        short imm_val = inst_buf[5] << 8 | (unsigned char) inst_buf[4];
                        inst_ptr = sprintf(next_inst, "mov [%s + %d], word %d\n", mem_addresses[rm], disp, imm_val);
                    } else {
                        fread(&inst_buf[inst_size], sizeof(*inst_buf), 3, fp);
                        unsigned short disp = inst_buf[3] << 8 | (unsigned char) inst_buf[2];
                        char imm_val = inst_buf[4];
                        inst_ptr = sprintf(next_inst, "mov [%s + %d], byte %d\n", mem_addresses[rm], disp, imm_val);
                    }
                    fwrite(next_inst, sizeof(*next_inst), inst_ptr, asm_file);
                    inst_ptr = 0;
                    inst_size = 0;
                    w = false;
                    imm = false;
                } else if ((inst_buf[1] & 0b11000000) == 0b01000000) {
                    // Immediate to memory with 8-bit displacement
                    if (w) {
                        fread(&inst_buf[inst_size], sizeof(*inst_buf), 3, fp);
                        unsigned char disp = inst_buf[2];
                        short imm_val = inst_buf[4] << 8 | (unsigned char) inst_buf[3];
                        inst_ptr = sprintf(next_inst, "mov [%s + %d], word %d\n", mem_addresses[rm], disp, imm_val);
                    } else {
                        fread(&inst_buf[inst_size], sizeof(*inst_buf), 2, fp);
                        unsigned char disp = inst_buf[2];
                        char imm_val = inst_buf[3];
                        inst_ptr = sprintf(next_inst, "mov [%s + %d], byte %d\n", mem_addresses[rm], disp, imm_val);
                    }
                    fwrite(next_inst, sizeof(*next_inst), inst_ptr, asm_file);
                    inst_ptr = 0;
                    inst_size = 0;
                    w = false;
                    imm = false;
                } else if ((inst_buf[1] & 0b11000000) == 0) {
                    // Immediate to memory with no displacement
                    if (w) {
                        fread(&inst_buf[inst_size], sizeof(*inst_buf), 2, fp);
                        short imm_val = inst_buf[3] << 8 | (unsigned char) inst_buf[2];
                        inst_ptr = sprintf(next_inst, "mov [%s], word %d\n", mem_addresses[rm], imm_val);
                    } else {
                        fread(&inst_buf[inst_size], sizeof(*inst_buf), 1, fp);
                        char imm_val = inst_buf[2];
                        inst_ptr = sprintf(next_inst, "mov [%s], byte %d\n", mem_addresses[rm], imm_val);
                    }
                    fwrite(next_inst, sizeof(*next_inst), inst_ptr, asm_file);
                    inst_ptr = 0;
                    inst_size = 0;
                    w = false;
                    imm = false;
                }


            }
        } 
    }
    return 0;
}