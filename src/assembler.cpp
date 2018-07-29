/* ASSEMBLER
 * Assembler object
 *
 * Stefan Wong 2018
 */

#include <iostream>
#include <iomanip>
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

inline uint16_t Assembler::asm_of6(const uint16_t arg)
{
    return 0x0000 | (arg & 0x001F);
}

inline uint16_t Assembler::asm_pc9(const uint16_t arg)
{
    return 0x0000 | (arg & 0x01FF);
}

// Instruction assembly
void Assembler::asm_add(const LineInfo& line)
{
    Instr instr;
    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] (src line " 
            << line.line_num << ") assembling ADD" << std::endl;
    }
    instr.ins = (instr.ins | this->asm_arg1(line.arg1));
    instr.ins = (instr.ins | this->asm_arg2(line.arg2));
    instr.ins = (instr.ins | this->asm_arg3(line.arg3));
    if(line.is_imm)
        instr.ins = (instr.ins | (1 << 5));
    instr.ins  = (instr.ins | (line.opcode.opcode << 12));
    // TODO : proper address resolution
    instr.adr = line.addr;

    this->program.add(instr);
}

void Assembler::asm_and(const LineInfo& line)
{
    Instr instr;
    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] (src line " 
            << line.line_num << ") assembling AND" << std::endl;
    }
    instr.ins = (instr.ins | (line.opcode.opcode << 12));
    instr.ins = (instr.ins| this->asm_arg1(line.arg1));
    instr.ins = (instr.ins| this->asm_arg2(line.arg2));
    instr.ins = (instr.ins | this->asm_arg3(line.arg3));
    if(line.is_imm)
        instr.ins = (instr.ins | (1 << 5));
    // TODO : proper address resolution
    instr.adr = line.addr;

    this->program.add(instr);
}

void Assembler::asm_br(const LineInfo& line)
{
    Instr instr;
    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] (src line " 
            << line.line_num << ") assembling BR" << std::endl;
    }
    instr.ins = (instr.ins | (line.opcode.opcode << 12));
    instr.ins = (instr.ins | (line.flags << 9));
    instr.ins = (instr.ins | (line.imm & 0x01FF));
    instr.adr = line.addr;

    this->program.add(instr);
}

void Assembler::asm_ld(const LineInfo& line)
{
    Instr instr;
    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] (src line " 
            << line.line_num << ") assembling LD" << std::endl;
    }
    instr.ins = (instr.ins | (line.opcode.opcode << 12));
    instr.ins = (instr.ins | this->asm_arg1(line.arg1));
    instr.ins = (instr.ins | this->asm_pc9(line.imm));
    instr.adr = line.addr;

    this->program.add(instr);
}

void Assembler::asm_ldr(const LineInfo& line)
{
    Instr instr;
    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] (src line " 
            << line.line_num << ") assembling LDR" << std::endl;
    }
    instr.ins = (instr.ins | (line.opcode.opcode << 12));
    instr.ins = (instr.ins | this->asm_arg1(line.arg1));
    instr.ins = (instr.ins | this->asm_of6(line.imm));
    instr.adr = line.addr;

    this->program.add(instr);
}

void Assembler::asm_lea(const LineInfo& line)
{
    Instr instr;
    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] (src line " 
            << line.line_num << ") assembling LEA" << std::endl;
    }
    instr.ins = (instr.ins | (line.opcode.opcode << 12));
    instr.ins = (instr.ins | this->asm_arg1(line.arg1));
    instr.ins = (instr.ins | this->asm_pc9(line.imm));
    instr.adr = line.addr;

    this->program.add(instr);
}

void Assembler::asm_str(const LineInfo& line)
{
    Instr instr;
    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] (src line " 
            << line.line_num << ") assembling STR" << std::endl;
    }
    instr.ins = (instr.ins | (line.opcode.opcode << 12));
    instr.ins = (instr.ins | this->asm_arg1(line.arg1));
    instr.ins = (instr.ins | this->asm_arg2(line.arg2));
    instr.ins = (instr.ins | this->asm_of6(line.imm));
    instr.adr = line.addr;

    this->program.add(instr);
}


// Go through the SourceInfo and update addresses
void Assembler::resolveLabels(void)
{
    if(this->src_info.getNumLines() == 0)
        return;

    unsigned int idx;
    LineInfo line;
    for(idx = 0; idx < this->src_info.getNumLines(); ++idx)
    {
        line = this->src_info.get(idx);
        if(line.label !=  "\0")
        {
            // find the label in the source info
        }
    }
}

void Assembler::assemble(void)
{
    LineInfo cur_line;
    unsigned int num_lines, idx;

    // First, resolve label addresses

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
                std::cout << "[" << __FUNCTION__ << "] (src line " << 
                    cur_line.line_num << ") is directive " <<
                    cur_line.symbol << std::endl;
            }
            continue;
        }

        // Handle opcodes 
        switch(cur_line.opcode.opcode)
        {
            case LC3_ADD:
                this->asm_add(cur_line);
                break;
            case LC3_AND:
                this->asm_and(cur_line);
                break;
            case LC3_BR:
                this->asm_br(cur_line);
                break;
            case LC3_LEA:
                this->asm_lea(cur_line);
                break;
            case LC3_LD:
                this->asm_ld(cur_line);
                break;
            case LC3_LDR:
                this->asm_ldr(cur_line);
                break;
            case LC3_STR:
                this->asm_str(cur_line);
                //std::cout << "Got STR (line " << 
                //    cur_line.line_num << "), not implemented" << std::endl;
                break;
            default:
                std::cout << "[" << __FUNCTION__ <<
                    "] (line " << cur_line.line_num << 
                    ") invalid opcode 0x" << std::hex << 
                    std::setw(2) << std::setfill('0') <<
                    std::uppercase << " with mnemonic " <<
                    cur_line.opcode.mnemonic << std::endl;
                break;
        }
    }
}

unsigned int Assembler::getNumErr(void) const
{
    return this->num_err;
}

AsmProg Assembler::getProgram(void) const
{
    return this->program;
}

std::vector<Instr> Assembler::getInstrs(void) const
{
    return this->program.getInstr();
}



void Assembler::setVerbose(const bool v)
{
    this->verbose = v;
}

bool Assembler::getVerbose(void) const
{
    return this->verbose;
}


