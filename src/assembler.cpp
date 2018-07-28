/* ASSEMBLER
 * Assembler object
 *
 * Stefan Wong 2018
 */

#include <iostream>
#include "assembler.hpp"
// TODO ; also need LC3 constants here ...
#include "lc3.hpp"

Assembler::Assembler(const SourceInfo& si)
{
    this->src_info = si;
    this->num_err = 0;
    this->verbose = false;
}

Assembler::~Assembler() {}

inline uint16_t Assembler::asm_arg1(const uint16_t arg)
{
    return 0x0000 | (arg << 9);
}

inline uint16_t Assembler::asm_arg2(const uint16_t arg)
{
    return 0x0000 | (arg << 6);
}

inline uint16_t Assembler::asm_arg3(const uint16_t arg)
{
    return 0x0000 | (arg);
}

// Instruction assembly
void Assembler::asm_add(const LineInfo& line)
{
    uint16_t instr = 0;
    if(this->verbose)
        std::cout << "[" << __FUNCTION__ << "] assembling ADD" << std::endl;
    instr = (instr | this->asm_arg1(line.arg1));
    instr = (instr | this->asm_arg2(line.arg2));
    instr = (instr | this->asm_arg3(line.arg3));
    if(line.is_imm)
        instr = (instr | (1 << 5));
    instr  = (instr | (line.opcode.opcode << 12));

    this->instrs.push_back(instr);
}


void Assembler::assemble(void)
{
    LineInfo cur_line;
    unsigned int num_lines, idx;

    this->num_err = 0;
    num_lines = this->src_info.getNumLines();
    for(idx = 0; idx < num_lines; idx++)
    {
        cur_line = this->src_info.get(idx);
        if(cur_line.error)
        {
            this->num_err++;
            continue;
        }
        // Handle directives 
        if(cur_line.is_directive)
        {
            if(this->verbose)
            {
                std::cout << "[" << __FUNCTION__ << "] (line " << 
                    cur_line.line_num << " is directive " <<
                    cur_line.symbol << std::endl;
            }
            continue;
        }


        // Handle opcodes 
        switch(cur_line.opcode.opcode)
        {
            case LC3_ADD:
                this->asm_add(cur_line);
        }
    }
}

unsigned int Assembler::getNumErr(void) const
{
    return this->num_err;
}

std::vector<uint16_t> Assembler::getInstrs(void) const
{
    return this->instrs;
}

void Assembler::setVerbose(const bool v)
{
    this->verbose = v;
}


bool Assembler::getVerbose(void) const
{
    return this->verbose;
}
