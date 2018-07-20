/* BASIC DISASSEMBLER
 * This is a basic disassm loop for the LC3 instruction set
 *
 * Stefan Wong 2018
 */
#include <stdio.h>
#include <stdint.h>
#include "opcode.h"

int check_imm(uint16_t instr)
{
    return (instr & 0x0020) ? 1 : 0;
}

inline uint8_t dis_op1(uint16_t code)
{
    return (code & 0x0E00) >> 8;
}

inline uint8_t dis_op2(uint16_t code)
{
    return (code & 0x01E0) >> 5;
}

inline uint8_t dis_op3(uint16_t code)
{
    return (code & 0x0006) >> 8;
}

inline uint8_t dis_imm(uint16_t code)
{
    return (code & 0x000F);
}

inline uint8_t dis_offset(uint16_t code)
{
    return (code & 0x00FF);
}

inline uint8_t dis_jmp(uint16_t code)
{
    return (code & 0x01E0) >> 5;
}

inline uint8_t dis_instr(uint16_t code)
{
    return (code & 0xE000) >> 8;
}

inline uint16_t dis_pc11(uint16_t code)
{
    return (code & 0x0EFF);
}

inline uint16_t dis_pc9(uint16_t code)
{
    return (code & 0x01FF);
}

// TODO : Do I need to return anything?
void dis_lc3_op(uint16_t *codebuf, int pc)
{
    uint16_t *code = &codebuf[pc];

    fprintf(stdout, "%04X", pc);
    uint8_t instr = dis_instr(*code);
    switch(instr)
    {
        case LC3_ADD:
            if(check_imm(*code))
            {
                fprintf(stdout, "ADDI   : %02X <- #%02X,#%02X",
                        dis_op1(*code), dis_op2(*code), dis_imm(*code));
            }
            else
            {
                fprintf(stdout, "ADD    : %02X <- #%02X,#%02X",
                        dis_op1(*code), dis_op2(*code), dis_op3(*code));
            }
            break;

        case LC3_AND:
            if(check_imm(*code))
            {
                fprintf(stdout, "ANDI   : %02X <- #%02X,#%02X",
                        dis_op1(*code), dis_op2(*code), dis_imm(*code));
            }
            else
            {
                fprintf(stdout, "AND    : %02X <- #%02X,#%02X",
                        dis_op1(*code), dis_op2(*code), dis_op3(*code));
            }
            break;

        // TODO : do branch last (to get flags printed correctly)
        case LC3_BR:
            fprintf(stdout, "BR     : %02X <- #%02X,#%02X",
                    dis_op1(*code), dis_op2(*code), dis_op3(*code));
            break;
        //case LC3_JMP:
        //    fprintf(stdout, "JMP    : -> %02X", dis_jmp(*code));
        //    break;

        case LC3_JSR:
            if(instr & 0xE100)
            {
                fprintf(stdout, "JSR    : -> %03X", dis_pc11(*code));
            }
            else
            {
                fprintf(stdout, "JSRR   : -> %02X", dis_jmp(*code));
            }
            break;

        case LC3_LD:
            fprintf(stdout, "LD     : %02X <- %03X", dis_op1(*code), 
                    dis_pc9(*code));
            break;

        case LC3_LDI:
            fprintf(stdout, "LDI    : %02X <- %03X", dis_op1(*code), 
                    dis_pc9(*code));
            break;
            
        case LC3_LEA:
            fprintf(stdout, "LEA    : %02X <- %03X", dis_op1(*code), 
                    dis_pc9(*code));
            break;

        case LC3_NOT:
            fprintf(stdout, "NOT    : %02X <- %03X", dis_op1(*code), 
                    dis_op2(*code));
            break;

        //case LC3_RET:
        //    fprintf(stdout, "RET    : %02X <- %03X", dis_op1(*code), 
        //            dis_op2(*code));
        //    break;

        case LC3_RTI:
            fprintf(stdout, "RTI    ");
            break;

        case LC3_ST:
            fprintf(stdout, "ST     : %02X, %03X ", dis_op1(*code),
                    dis_pc9(*code));
            break;

        case LC3_STI:
            fprintf(stdout, "STI    : %02X, %03X ", dis_op1(*code),
                    dis_pc9(*code));
            break;

        case LC3_STR:
            fprintf(stdout, "STR    : %02X <- %02X, %03X ", dis_op1(*code),
                    dis_op2(*code), (*code & 0x003F));
            break;

        case LC3_TRAP:
            fprintf(stdout, "TRAP   : %02X", (*code & 0x003F));
            break;

        case LC3_RES:
            fprintf(stdout, "RES ");
            break;
        default:
            fprintf(stderr, "Invalid instruction %02X", (*code & 0xE0000) >> 8);
    }

    fprintf(stdout, "\n");
}
