/* DECODE
 * Instruction decode for LC-3
 *
 * Stefan Wong 2018
 */

#ifndef __DECODE_H
#define __DECODE_H

#include <stdint.h>
#include "isa.h"

static inline uint8_t get_opcode(uint16_t instr)
{
    return (instr & 0xF000) >> 8;
}

static inline int is_imm(uint16_t instr)
{
    return (instr & 0x0020) ? 1 : 0;
}

static inline int is_jmp(uint16_t instr)
{
    return ((instr & 0x01C0) == 0x7) ? 0 : 1;
}

static inline uint8_t get_dr(uint16_t instr)
{
    return (instr & 0x0E00) >> 9;
}

static inline uint8_t get_sr1(uint16_t instr)
{
    return (instr & 0x01E0) >> 6;
}

static inline uint8_t get_sr2(uint16_t instr)
{
    return (instr & 0x0007);
}

static inline uint8_t get_imm5(uint16_t instr)
{
    return (instr & 0x001F);
}

static inline uint8_t get_br(uint16_t instr)
{
    return (instr & 0x01E00) >> 6;
}


// Decode an instruction
void lc3_decode_instr(LC3State* state, uint16_t instr);


#endif /*__DECODE_H*/
