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
void regMem(const char *, bool, bool);
void immCheck(const char *, bool, bool=false);
void accumCheck(const char *, bool);
bool jumpCheck();
char inst_buf[MAX_INST_SIZE];
char next_inst[100];
size_t inst_ptr = 0;
size_t inst_size = 0;
FILE *fp;
FILE *asm_file;
int main(int argc, char *argv[]) {
    fp = fopen(argv[1], "rb");
    asm_file = fopen(argv[2], "wb");
    fwrite("bits 16\n", 1, 8, asm_file);

    // Read first byte of instruction
    while (fread(inst_buf, sizeof(*inst_buf), 1, fp)) {
        inst_size++; 
        char *op;
        bool regmem = false;
        bool accum = false;
        bool jump = jumpCheck();
        if (jump) {
            fwrite(next_inst, sizeof(*next_inst), inst_ptr, asm_file);
            inst_ptr = 0;
            inst_size = 0;
            continue;
        }
        switch (inst_buf[0] & 0b11111100) {
            case 0:
                op = "add";
                regmem = true;
                break;
            case 0b101000:
                op = "sub";
                regmem = true;
                break;
            case 0b111000:
                op = "cmp";
                regmem = true;
                break;
        }
        if (regmem) {
            bool d = inst_buf[0] & (1 << 1);
            bool w = inst_buf[0] & 1;
            fread(&inst_buf[inst_size], sizeof(*inst_buf), 1, fp); 
            inst_size++;
            regMem(op, d, w);
        } else if ((inst_buf[0] & 0b11111100) == 0b10000000) {
            bool s = inst_buf[0] & 0b10;
            bool w = inst_buf[0] & 1;
            fread(&inst_buf[inst_size], sizeof(*inst_buf), 1, fp); 
            inst_size++;
            switch ((inst_buf[1] & 0b00111000) >> 3) {
                case 0:
                    op = "add";
                    break;
                case 0b101:
                    op = "sub";
                    break;
                case 0b111:
                    op = "cmp";
                    break;
            }
            immCheck(op, w, s);
        } else {
            switch (inst_buf[0] & 0b11111110) {
                case 0b100:
                    op = "add";
                    accum = true;
                    break;
                case 0b101100:
                    op = "sub";
                    accum = true;
                    break;
                case 0b111100:
                    op = "cmp";
                    accum = true;
                    break;
            }
            if (accum) {
                bool w = inst_buf[0] & 1;
                accumCheck(op, w);
            }
        } 
    }
    return 0;
}

bool jumpCheck() {
    switch ((unsigned char) inst_buf[0]) {
        case 0b01110100: // JE/JZ
            fread(&inst_buf[inst_size], sizeof(*inst_buf), 1, fp);
            inst_ptr = sprintf(next_inst, "je %d\n", inst_buf[1]);
            return true;
        case 0b01111100: // JL/JNGE
            fread(&inst_buf[inst_size], sizeof(*inst_buf), 1, fp);
            inst_ptr = sprintf(next_inst, "jl %d\n", inst_buf[1]);
            return true;
        case 0b01111110: // JLE/JNG
            fread(&inst_buf[inst_size], sizeof(*inst_buf), 1, fp);
            inst_ptr = sprintf(next_inst, "jle %d\n", inst_buf[1]);
            return true;
        case 0b01110010: // JB/JNAE
            fread(&inst_buf[inst_size], sizeof(*inst_buf), 1, fp);
            inst_ptr = sprintf(next_inst, "jb %d\n", inst_buf[1]);
            return true;
        case 0b01110110: // JBE/JNA
            fread(&inst_buf[inst_size], sizeof(*inst_buf), 1, fp);
            inst_ptr = sprintf(next_inst, "jbe %d\n", inst_buf[1]);
            return true;
        case 0b01111010: // JP/JPE
            fread(&inst_buf[inst_size], sizeof(*inst_buf), 1, fp);
            inst_ptr = sprintf(next_inst, "jp %d\n", inst_buf[1]);
            return true;
        case 0b01110000: // JO
            fread(&inst_buf[inst_size], sizeof(*inst_buf), 1, fp);
            inst_ptr = sprintf(next_inst, "jo %d\n", inst_buf[1]);
            return true;
        case 0b01111000: // JS
            fread(&inst_buf[inst_size], sizeof(*inst_buf), 1, fp);
            inst_ptr = sprintf(next_inst, "js %d\n", inst_buf[1]);
            return true;
        case 0b01110101: // JNE/JNZ
            fread(&inst_buf[inst_size], sizeof(*inst_buf), 1, fp);
            inst_ptr = sprintf(next_inst, "jne %d\n", inst_buf[1]);
            return true;
        case 0b01111101: // JNL/JGE
            fread(&inst_buf[inst_size], sizeof(*inst_buf), 1, fp);
            inst_ptr = sprintf(next_inst, "jnl %d\n", inst_buf[1]);
            return true;
        case 0b01111111: // JNLE/JG
            fread(&inst_buf[inst_size], sizeof(*inst_buf), 1, fp);
            inst_ptr = sprintf(next_inst, "jnle %d\n", inst_buf[1]);
            return true;
        case 0b01110011: // JNB/JAE
            fread(&inst_buf[inst_size], sizeof(*inst_buf), 1, fp);
            inst_ptr = sprintf(next_inst, "jnb %d\n", inst_buf[1]);
            return true;
        case 0b01110111: // JNBE/JA
            fread(&inst_buf[inst_size], sizeof(*inst_buf), 1, fp);
            inst_ptr = sprintf(next_inst, "jnbe %d\n", inst_buf[1]);
            return true;
        case 0b01111011: // JNP/JPO
            fread(&inst_buf[inst_size], sizeof(*inst_buf), 1, fp);
            inst_ptr = sprintf(next_inst, "jnp %d\n", inst_buf[1]);
            return true;
        case 0b01110001: // JNO
            fread(&inst_buf[inst_size], sizeof(*inst_buf), 1, fp);
            inst_ptr = sprintf(next_inst, "jno %d\n", inst_buf[1]);
            return true;
        case 0b01111001: // JNS
            fread(&inst_buf[inst_size], sizeof(*inst_buf), 1, fp);
            inst_ptr = sprintf(next_inst, "jns %d\n", inst_buf[1]);
            return true;
        case 0b11100010: // LOOP
            fread(&inst_buf[inst_size], sizeof(*inst_buf), 1, fp);
            inst_ptr = sprintf(next_inst, "loop %d\n", inst_buf[1]);
            return true;
        case 0b11100001: // LOOPZ/LOOPE
            fread(&inst_buf[inst_size], sizeof(*inst_buf), 1, fp);
            inst_ptr = sprintf(next_inst, "loopz %d\n", inst_buf[1]);
            return true;
        case 0b11100000: // LOOPNZ/LOOPNE
            fread(&inst_buf[inst_size], sizeof(*inst_buf), 1, fp);
            inst_ptr = sprintf(next_inst, "loopnz %d\n", inst_buf[1]);
            return true;
        case 0b11100011: // JCXZ
            fread(&inst_buf[inst_size], sizeof(*inst_buf), 1, fp);
            inst_ptr = sprintf(next_inst, "jcxz %d\n", inst_buf[1]);
            return true;
    }
    return false;
}

void accumCheck(const char *op, bool w) {
    if (w) {
        fread(&inst_buf[inst_size], sizeof(*inst_buf), 2, fp);
        short imm_val = inst_buf[2] << 8 | (unsigned char) inst_buf[1];
        inst_ptr = sprintf(next_inst, "%s ax, %d\n", op, imm_val);
    } else {
        fread(&inst_buf[inst_size], sizeof(*inst_buf), 1, fp);
        char imm_val = inst_buf[1];
        inst_ptr = sprintf(next_inst, "%s al, %d\n", op, imm_val);
    }
    fwrite(next_inst, sizeof(*next_inst), inst_ptr, asm_file);
    inst_ptr = 0;
    inst_size = 0;
}
void immCheck(const char *op, bool w, bool s) {
    char rm = inst_buf[1] & 0b111;
    // Immediate to reg/mem
    if ((inst_buf[1] & 0b11000000) == 0b11000000) {
        // Immediate to register
        if (!s && w) {
            fread(&inst_buf[inst_size], sizeof(*inst_buf), 2, fp);
            inst_size += 2;
            short imm_val = inst_buf[3] << 8 | (unsigned char) inst_buf[2];
            inst_ptr = sprintf(next_inst, "%s %s, %d\n", op, reg_arr[rm + (w << 3)], imm_val);
        } else {
            fread(&inst_buf[inst_size], sizeof(*inst_buf), 1, fp);
            inst_size++;
            char imm_val = inst_buf[2];
            inst_ptr = sprintf(next_inst, "%s %s, %d\n", op, reg_arr[rm + (w << 3)], imm_val);
        }
        fwrite(next_inst, sizeof(*next_inst), inst_ptr, asm_file);
        inst_ptr = 0;
        inst_size = 0;
    } else if ((inst_buf[1] & 0b11000000) == 0b10000000) {
        // Immediate to memory with 16-bit displacement
        if (!s && w) {
            fread(&inst_buf[inst_size], sizeof(*inst_buf), 4, fp);
            unsigned short disp = inst_buf[3] << 8 | (unsigned char) inst_buf[2];
            short imm_val = inst_buf[5] << 8 | (unsigned char) inst_buf[4];
            if (strncmp("mov", op, 3) == 0) {
                inst_ptr = sprintf(next_inst, "%s [%s + %d], word %d\n", op, mem_addresses[rm], disp, imm_val);
            } else {
                inst_ptr = sprintf(next_inst, "%s word [%s + %d], %d\n", op, mem_addresses[rm], disp, imm_val);
            }
        } else {
            fread(&inst_buf[inst_size], sizeof(*inst_buf), 3, fp);
            unsigned short disp = inst_buf[3] << 8 | (unsigned char) inst_buf[2];
            char imm_val = inst_buf[4];
            if (strncmp("mov", op, 3) == 0) {
                inst_ptr = sprintf(next_inst, "%s [%s + %d], byte %d\n", op, mem_addresses[rm], disp, imm_val);
            } else {
                const char *size = w ? "word" : "byte";
                inst_ptr = sprintf(next_inst, "%s %s [%s + %d], %d\n", op, size, mem_addresses[rm], disp, imm_val);
            }
        }
        fwrite(next_inst, sizeof(*next_inst), inst_ptr, asm_file);
        inst_ptr = 0;
        inst_size = 0;
    } else if ((inst_buf[1] & 0b11000000) == 0b01000000) {
        // Immediate to memory with 8-bit displacement
        if (!s && w) {
            fread(&inst_buf[inst_size], sizeof(*inst_buf), 3, fp);
            unsigned char disp = inst_buf[2];
            short imm_val = inst_buf[4] << 8 | (unsigned char) inst_buf[3];
            if (strncmp("mov", op, 3) == 0) {
                inst_ptr = sprintf(next_inst, "%s [%s + %d], word %d\n", op, mem_addresses[rm], disp, imm_val);
            } else {
                inst_ptr = sprintf(next_inst, "%s word [%s + %d], %d\n", op, mem_addresses[rm], disp, imm_val);
            }
        } else {
            fread(&inst_buf[inst_size], sizeof(*inst_buf), 2, fp);
            unsigned char disp = inst_buf[2];
            char imm_val = inst_buf[3];
            if (strncmp("mov", op, 3) == 0) {
                inst_ptr = sprintf(next_inst, "%s [%s + %d], byte %d\n", op, mem_addresses[rm], disp, imm_val);
            } else {
                const char *size = w ? "word" : "byte";
                inst_ptr = sprintf(next_inst, "%s %s [%s + %d], %d\n", op, size, mem_addresses[rm], disp, imm_val);
            }
        }
        fwrite(next_inst, sizeof(*next_inst), inst_ptr, asm_file);
        inst_ptr = 0;
        inst_size = 0;
    } else if ((inst_buf[1] & 0b11000000) == 0) {
        // Immediate to memory with no displacement
        if (rm == 0b110) {
            // Direct address
            fread(&inst_buf[inst_size], sizeof(*inst_buf), 2, fp);
            inst_size += 2;
            unsigned short mem_addr = inst_buf[3] << 8 | (unsigned char) inst_buf[2];
            if (!s && w) {
                fread(&inst_buf[inst_size], sizeof(*inst_buf), 2, fp);
                inst_size += 2;
                short imm_val = inst_buf[5] << 8 | (unsigned char) inst_buf[4];
                inst_ptr = sprintf(next_inst, "%s word [%d], %d\n", op, mem_addr, imm_val);
            } else if (w) {
                fread(&inst_buf[inst_size], sizeof(*inst_buf), 1, fp);
                inst_size++;
                char imm_val = inst_buf[4];
                inst_ptr = sprintf(next_inst, "%s word [%d], %d\n", op, mem_addr, imm_val);
            } else {
                fread(&inst_buf[inst_size], sizeof(*inst_buf), 1, fp);
                inst_size++;
                char imm_val = inst_buf[4];
                inst_ptr = sprintf(next_inst, "%s byte [%d], %d\n", op, mem_addr, imm_val);
            }
        } else if (!s && w) {
            fread(&inst_buf[inst_size], sizeof(*inst_buf), 2, fp);
            short imm_val = inst_buf[3] << 8 | (unsigned char) inst_buf[2];
            inst_ptr = sprintf(next_inst, "%s [%s], word %d\n", op, mem_addresses[rm], imm_val);
            if (strncmp("mov", op, 3) == 0) {
                inst_ptr = sprintf(next_inst, "%s [%s], word %d\n", op, mem_addresses[rm], imm_val);
            } else {
                inst_ptr = sprintf(next_inst, "%s word [%s], %d\n", op, mem_addresses[rm], imm_val);
            }
        } else {
            fread(&inst_buf[inst_size], sizeof(*inst_buf), 1, fp);
            char imm_val = inst_buf[2];
            inst_ptr = sprintf(next_inst, "%s [%s], byte %d\n", op, mem_addresses[rm], imm_val);
            if (strncmp("mov", op, 3) == 0) {
                inst_ptr = sprintf(next_inst, "%s [%s], byte %d\n", op, mem_addresses[rm], imm_val);
            } else {
                const char *size = w ? "word" : "byte";
                inst_ptr = sprintf(next_inst, "%s %s [%s], %d\n", op, size, mem_addresses[rm], imm_val);
            }
        }
        fwrite(next_inst, sizeof(*next_inst), inst_ptr, asm_file);
        inst_ptr = 0;
        inst_size = 0;
    }
}

void regMem(const char *op, bool d, bool w) {
    char rm = inst_buf[1] & 0b111;
    char reg = (inst_buf[1] & 0b00111000) >> 3;

    if ((inst_buf[1] & 0b11000000) == 0b11000000) {
        // Reg to reg
        char r1, r2;
        if (d) {
            r1 = reg;
            r2 = rm;
        } else {
            r1 = rm;
            r2 = reg;
        }

        inst_ptr = sprintf(next_inst, "%s %s, %s\n", op, reg_arr[r1 + (w << 3)], reg_arr[r2 + (w << 3)]);
        fwrite(next_inst, sizeof(*next_inst), inst_ptr, asm_file);
        inst_ptr = 0;
        inst_size = 0;

    } else if ((inst_buf[1] & 0b11000000) == 0b10000000) {
        // 16-bit displacement
        fread(&inst_buf[inst_size], sizeof(*inst_buf), 2, fp);
        inst_size += 2;
        short disp = inst_buf[3] << 8 | (unsigned char) inst_buf[2];
        if (d) {
            inst_ptr = sprintf(next_inst, "%s %s, [%s + %d]\n", op, reg_arr[reg + (w << 3)], mem_addresses[rm], disp);
        } else {
            inst_ptr = sprintf(next_inst, "%s [%s + %d], %s\n", op, mem_addresses[rm], disp, reg_arr[reg + (w << 3)]);
        }

        fwrite(next_inst, sizeof(*next_inst), inst_ptr, asm_file);
        inst_ptr = 0;
        inst_size = 0;
        
    } else if ((inst_buf[1] & 0b11000000) == 0b01000000) {
        // 8-bit displacement
        fread(&inst_buf[inst_size], sizeof(*inst_buf), 1, fp);
        inst_size++;
        char disp = inst_buf[2];
        if (d) {
            inst_ptr = sprintf(next_inst, "%s %s, [%s + %d]\n", op, reg_arr[reg + (w << 3)], mem_addresses[rm], disp);
        } else {
            inst_ptr = sprintf(next_inst, "%s [%s + %d], %s\n", op, mem_addresses[rm], disp, reg_arr[reg + (w << 3)]);
        }

        fwrite(next_inst, sizeof(*next_inst), inst_ptr, asm_file);
        inst_ptr = 0;
        inst_size = 0;
                  
    } else {
        // No displacement
        if (rm == 0b110) {
            // Direct address, instruction ends
            if (w) {
                fread(&inst_buf[inst_size], sizeof(*inst_buf), 2, fp);
                unsigned short disp = inst_buf[3] << 8 | (unsigned char) inst_buf[2];

                if (d) {
                    inst_ptr = sprintf(next_inst, "%s %s, [%d]\n", op, reg_arr[reg + (w << 3)], disp);
                } else {
                    inst_ptr = sprintf(next_inst, "%s [%d], %s\n", op, disp, reg_arr[reg + (w << 3)]);
                }
            } else {
                fread(&inst_buf[inst_size], sizeof(*inst_buf), 1, fp);
                unsigned char disp = inst_buf[2];

                if (d) {
                    inst_ptr = sprintf(next_inst, "%s %s, [%d]\n", op, reg_arr[reg + (w << 3)], disp);
                } else {
                    inst_ptr = sprintf(next_inst, "%s [%d], %s\n", op, disp, reg_arr[reg + (w << 3)]);
                }
            } 
            fwrite(next_inst, sizeof(*next_inst), inst_ptr, asm_file);
            inst_ptr = 0;
            inst_size = 0;
        } else {
            // No displacement, instruction ends
            if (d) {
                inst_ptr = sprintf(next_inst, "%s %s, [%s]\n", op, reg_arr[reg + (w << 3)], mem_addresses[rm]);
            } else {
                inst_ptr = sprintf(next_inst, "%s [%s], %s\n", op, mem_addresses[rm], reg_arr[reg + (w << 3)]);
            }
            fwrite(next_inst, sizeof(*next_inst), inst_ptr, asm_file);
            inst_ptr = 0;
            inst_size = 0;
        }
    }
}