/* DECODE
 * Instruction decode for LC-3
 *
 * Stefan Wong 2018
 */

#include "decode.h"
#include "opcode.h"

// First cut here is just to have a large case statement
// that creates a jump-table for each instruction

void lc3_decode_instr(LC3State* state, uint16_t instr)
{
    uint8_t op = get_opcode(instr);
    uint8_t dst;
    uint8_t sr1;
    uint8_t sr2;
    uint8_t br;
    uint8_t imm5;
    //uint16_t pc9;
    //uint16_t pc11;

    switch(op)
    {
        case LC3_ADD:
            dst = get_dr(instr);
            sr1 = get_sr1(instr);
            if(is_imm(instr))
            {
                imm5 = get_imm5(instr);
                state->gpr[dst] = state->gpr[sr1] + imm5;
            }
            else
            {
                sr2 = get_sr2(instr);
                state->gpr[dst] = state->gpr[sr1] + state->gpr[sr2];
            }
            break;

        case LC3_AND:
            dst = get_dr(instr);
            sr1 = get_sr1(instr);
            if(is_imm(instr))
            {
                imm5 = get_imm5(instr);
                state->gpr[dst] = state->gpr[sr1] & imm5;
            }
            else
            {
                sr2 = get_sr2(instr);
                state->gpr[dst] = state->gpr[sr1] & state->gpr[sr2];
            }
            break;

        case LC3_BR:
            break;

        case LC3_JMP_RET:
            if(is_jmp(instr))  // JMP 
            {
                br = get_br(instr);
                state->pc = state->gpr[br];
            }
            else            // RET
            {
                // TODO :
            }
            break;


    }
}


