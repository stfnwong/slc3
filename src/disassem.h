/* DISASSEM
 * Helper functions for disassembler
 *
 * Stefan Wong 2018
 */

#ifndef __DISASSEM_H
#define __DIASSEM_H

static inline int check_imm(uint16_t instr)
{
    return (instr & 0x0020) ? 1 : 0;
}

static inline uint8_t dis_op1(uint16_t code)
{
    return (code & 0x0E00) >> 8;
}

static inline uint8_t dis_op2(uint16_t code)
{
    return (code & 0x01E0) >> 5;
}

static inline uint8_t dis_op3(uint16_t code)
{
    return (code & 0x0006) >> 8;
}

static inline uint8_t dis_imm(uint16_t code)
{
    return (code & 0x000F);
}

static inline uint8_t dis_offset(uint16_t code)
{
    return (code & 0x00FF);
}

static inline uint8_t dis_jmp(uint16_t code)
{
    return (code & 0x01E0) >> 5;
}

static inline uint8_t dis_instr(uint16_t code)
{
    return (code & 0xE000) >> 8;
}

static inline uint16_t dis_pc11(uint16_t code)
{
    return (code & 0x0EFF);
}

static inline uint16_t dis_pc9(uint16_t code)
{
    return (code & 0x01FF);
}

#endif /*__DISASSEM_H*/
