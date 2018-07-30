/* DISASSEMBLER
 * An LC3 disassembler
 *
 * Stefan Wong 2018
 */

#include <iostream>
#include <iomanip>
#include <fstream>
#include "disassembler.hpp"
// TODO : again, need to factor out the LC3 specific 
// stuff at some future point
#include "lc3.hpp"


Disassembler::Disassembler()
{
    this->verbose = false;
}

Disassembler::~Disassembler() {} 

// ===== Instruction disassembly
inline bool Disassembler::is_imm(const uint16_t instr) const
{
    return (instr & 0x0020) ? true : false;
}

inline uint8_t Disassembler::dis_op1(const uint16_t instr) const
{
    return (instr & 0x0E00) >> 8;
}

inline uint8_t Disassembler::dis_op2(const uint16_t instr) const
{
    return (instr & 0x01E0) >> 5;
}

inline uint8_t Disassembler::dis_op3(const uint16_t instr) const
{
    return (instr & 0x0006);
}

inline uint8_t Disassembler::dis_flags(const uint16_t instr) const
{
    return (instr & 0x0E00) >> 8;
}

inline uint8_t Disassembler::dis_opcode(const uint16_t instr) const
{
    return (instr & 0xF000) >> 11;
}

inline uint8_t Disassembler::dis_of6(const uint16_t instr) const
{
    return (instr & 0x003F);
}

inline uint16_t Disassembler::dis_pc9(const uint16_t instr) const
{
    return (instr & 0x1FF);
}

inline uint16_t Disassembler::dis_pc11(const uint16_t instr) const
{
    return (instr & 0x0EFF);

}


// File load 
int Disassembler::read(const std::string& filename)
{
    int status; 
    return status = this->program.read(filename);
}

int Disassembler::disInstr(const Instr& instr)
{
    int status = 0;
    Opcode o;

    // TODO : Look up menmonic
    o.opcode = this->dis_opcode(instr.ins);
    this->cur_line.opcode = o;
    this->cur_line.addr   = instr.adr;
    switch(o.opcode)
    {
        case LC3_ADD:
            this->cur_line.arg1 = this->dis_op1(instr.ins);
            this->cur_line.arg2 = this->dis_op2(instr.ins);
            this->cur_line.is_imm  = this->is_imm(instr.ins);
            if(this->cur_line.is_imm)
                this->cur_line.imm = this->dis_op3(instr.ins);
            else
                this->cur_line.arg3 = this->dis_op3(instr.ins);

            break;

        case LC3_AND:
            this->cur_line.arg1 = this->dis_op1(instr.ins);
            this->cur_line.arg2 = this->dis_op2(instr.ins);
            this->cur_line.is_imm  = this->is_imm(instr.ins);
            if(this->cur_line.is_imm)
                this->cur_line.imm = this->dis_op3(instr.ins);
            else
                this->cur_line.arg3 = this->dis_op3(instr.ins);

            break;

        case LC3_JSR:
            this->cur_line.imm = this->dis_pc11(instr.ins);

            break;

        case LC3_BR:
            this->cur_line.flags = this->dis_flags(instr.ins);
            this->cur_line.imm   = this->dis_pc9(instr.ins);

            break;

        case LC3_LEA:
            this->cur_line.arg1 = this->dis_op1(instr.ins);
            this->cur_line.imm  = this->dis_pc9(instr.ins);

            break;

        case LC3_LD:
            this->cur_line.arg1 = this->dis_op1(instr.ins); 
            this->cur_line.imm  = this->dis_pc9(instr.ins);

            break;

        case LC3_NOT:
            this->cur_line.arg1 = this->dis_op1(instr.ins);
            this->cur_line.arg2 = this->dis_op2(instr.ins);
            this->cur_line.imm  = this->dis_of6(instr.ins);
            
            break;

        case LC3_STR:
            this->cur_line.arg1 = this->dis_op1(instr.ins);


            break;

        default:
            std::cout << "[" << __FUNCTION__ << "] invalid opcode $" <<
                std::uppercase << std::hex << std::setw(2) << 
                std::setfill('0') << o.opcode 
                << " at 0x" << instr.adr << std::endl;
            status = -1;
            break;
    }

    return status;
}

void Disassembler::disassemble(void)
{
    unsigned int i;

    if(this->program.getNumInstr() == 0)
    {
        if(this->verbose)
            std::cout << "[" << __FUNCTION__ << "] no instructions in program" << std::endl;
        return;
    }

    // Walk through the instructions and disassemble
    initLineInfo(this->cur_line);
    Instr instr;
    int status;
    for(i = 0; i < this->program.getNumInstr(); ++i)
    {
        instr = this->program.getInstr(i);
        // TODO : look up mnemonic
        status = this->disInstr(instr);
        if(status < 0)
        {
            std::cout << "[" << __FUNCTION__ << 
                "] Error disassembling instruction " << 
                std::dec << i+1 << "/" << 
                std::dec << this->program.getNumInstr() << std::endl;
        }
        this->source.add(this->cur_line);
    }
}

// Get stats
unsigned int Disassembler::numAsmInstr(void) const
{
    return this->program.getNumInstr();
}

unsigned int Disassembler::numSrcLines(void) const
{
    return this->source.getNumLines();
}

// How much text output to generate
void Disassembler::setVerbose(const bool v)
{
    this->verbose = v;
    this->program.setVerbose(this->verbose);
    //this->source.setVerbose(this->verbose);
}

bool Disassembler::getVerbose(void) const
{
    return this->verbose;
}

SourceInfo Disassembler::getSourceInfo(void) const
{
    return this->source;
}
