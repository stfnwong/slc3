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
    // Build the op table 
    for(const Opcode &op : lc3_op_list)
        this->lc3_op_table.add(op);
}

Disassembler::~Disassembler() {} 

// ===== Instruction disassembly
inline uint8_t Disassembler::dis_opcode(const uint16_t instr) const
{
    return (instr & 0xF000) >> 12;
}
inline bool Disassembler::is_imm(const uint16_t instr) const
{
    return (instr & 0x0020) ? true : false;
}
inline uint8_t Disassembler::dis_op1(const uint16_t instr) const
{
    return (instr & 0x0E00) >> 9;
}
inline uint8_t Disassembler::dis_op2(const uint16_t instr) const
{
    return (instr & 0x01E0) >> 6;
}
inline uint8_t Disassembler::dis_op3(const uint16_t instr) const
{
    return (instr & 0x0006);
}
inline uint8_t Disassembler::dis_flags(const uint16_t instr) const
{
    return (instr & 0x0E00) >> 9;
}
inline uint8_t Disassembler::dis_imm5(const uint16_t instr) const
{
    return (instr & 0x001F);
}
inline uint8_t Disassembler::dis_of6(const uint16_t instr) const
{
    return (instr & 0x003F);
}
inline uint16_t Disassembler::dis_pc9(const uint16_t instr) const
{
    return (instr & 0x01FF);
}
inline uint16_t Disassembler::dis_pc11(const uint16_t instr) const
{
    return (instr & 0x0EFF);
}
inline uint8_t Disassembler::dis_trap8(const uint16_t instr) const
{
    return (instr & 0x00FF);
}

// File load 
int Disassembler::read(const std::string& filename)
{
    int status; 
    return status = this->program.load(filename);
}

int Disassembler::disInstr(const Instr& instr)
{
    int status = 0;
    Opcode o;

    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << 
            "] disassembling instruction 0x" << std::hex <<
            std::setw(4) << std::setfill('0') << instr.ins <<
            " with address 0x" << std::setw(4) << std::setfill('0') <<
            instr.adr << std::endl;
    }

    o.opcode   = this->dis_opcode(instr.ins);
    o.mnemonic = this->lc3_op_table.getMnemonic(o.opcode);
    if(o.mnemonic == "OP_UNKNOWN")
    {
        std::cerr << "[" << __FUNCTION__ << "] cannot find mnemonic for opcode <0x" 
            << o.opcode << ">" << std::endl;
    }
    else
    {
        std::cout << "[" << __FUNCTION__ << "] found opcode 0x"
            << std::hex << std::setw(2) << std::right << o.opcode 
            << " with mnemonic " << o.mnemonic << std::endl;
    }
    this->cur_line.opcode = o;
    this->cur_line.addr   = instr.adr;
    switch(o.opcode)
    {
        case LC3_ADD:
            this->cur_line.opcode.mnemonic = "ADD";
            this->cur_line.arg1 = this->dis_op1(instr.ins);
            this->cur_line.arg2 = this->dis_op2(instr.ins);
            this->cur_line.is_imm  = this->is_imm(instr.ins);
            if(this->cur_line.is_imm)
                this->cur_line.imm = this->dis_imm5(instr.ins);
            else
                this->cur_line.arg3 = this->dis_op3(instr.ins);

            break;

        case LC3_AND:
            this->cur_line.opcode.mnemonic = "AND";
            this->cur_line.arg1 = this->dis_op1(instr.ins);
            this->cur_line.arg2 = this->dis_op2(instr.ins);
            this->cur_line.is_imm  = this->is_imm(instr.ins);
            if(this->cur_line.is_imm)
                this->cur_line.imm = this->dis_imm5(instr.ins);
            else
                this->cur_line.arg3 = this->dis_op3(instr.ins);

            break;

        case LC3_BR:
            this->cur_line.opcode.mnemonic = "BR";
            this->cur_line.flags = this->dis_flags(instr.ins);
            this->cur_line.imm   = this->dis_pc9(instr.ins);
            // Mnemonic here depends on what flags we have
            if(this->cur_line.flags & LC3_FLAG_N)
                this->cur_line.opcode.mnemonic += "n";
            if(this->cur_line.flags & LC3_FLAG_Z)
                this->cur_line.opcode.mnemonic += "z";
            if(this->cur_line.flags & LC3_FLAG_P)
                this->cur_line.opcode.mnemonic += "p";
            break;

        case LC3_JSR:
            this->cur_line.opcode.mnemonic = "JSR";
            this->cur_line.imm = this->dis_pc11(instr.ins);
            break;

        case LC3_LEA:
            this->cur_line.opcode.mnemonic = "LEA";
            this->cur_line.arg1 = this->dis_op1(instr.ins);
            this->cur_line.imm  = this->dis_pc9(instr.ins);
            break;

        case LC3_LD:
            this->cur_line.opcode.mnemonic = "LD";
            this->cur_line.arg1 = this->dis_op1(instr.ins); 
            this->cur_line.imm  = this->dis_pc9(instr.ins);

            break;

        case LC3_NOT:
            this->cur_line.opcode.mnemonic = "NOT";
            this->cur_line.arg1 = this->dis_op1(instr.ins);
            this->cur_line.arg2 = this->dis_op2(instr.ins);
            this->cur_line.imm  = this->dis_of6(instr.ins);
            
            break;

        case LC3_STR:
            this->cur_line.opcode.mnemonic = "STR";
            this->cur_line.arg1 = this->dis_op1(instr.ins);
            break;

        case LC3_TRAP:
            this->cur_line.imm = this->dis_trap8(instr.ins);
            break;

        default:
            std::cout << "[" << __FUNCTION__ << "] (line " << 
                std::dec << this->cur_line.line_num << ") invalid opcode $" <<
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
    this->line_ptr = 1;
    initLineInfo(this->cur_line);
    Instr instr;
    int status;
    for(i = 0; i < this->program.getNumInstr(); ++i)
    {
        instr  = this->program.getInstr(i);
        status = this->disInstr(instr);
        if(status < 0)
        {
            std::cout << "[" << __FUNCTION__ << 
                "] Error disassembling instruction " << 
                std::dec << i+1 << "/" << 
                std::dec << this->program.getNumInstr() << std::endl;
        }
        this->cur_line.line_num = this->line_ptr;
        this->source.add(this->cur_line);
        this->line_ptr++;
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

Program Disassembler::getProgram(void) const
{
    return this->program;
}

std::string Disassembler::line_to_asm(const LineInfo& l)
{
    std::ostringstream oss;

    if(l.is_label)
        oss << l.label << ":";
    oss << "\t " << l.opcode.mnemonic;
    switch(l.opcode.opcode)
    {
        case LC3_ADD:
        case LC3_AND:
            oss << " R" << std::dec << l.arg1 << ",";
            oss << " R" << std::dec << l.arg2 << ",";
            if(l.is_imm)
                oss << " $" << std::hex << std::setw(4) << std::left << l.imm;
            else
                oss << " R" << std::dec << l.arg3;
        break;

        case LC3_BR:
            if(l.flags & LC3_FLAG_N)
                oss << "n";
            if(l.flags & LC3_FLAG_Z)
                oss << "z";
            if(l.flags & LC3_FLAG_P)
                oss << "p";
            oss << " $" << std::hex << std::setw(4) << std::setfill('0') << l.imm;
            break;

        case LC3_LD:
        case LC3_LDI:
        case LC3_LEA:
            oss << " R" << std::dec << l.arg1 << ",";
            oss << " $" << std::hex << std::setw(4) << std::left << l.imm;
            break;

        case LC3_LDR:
            oss << " R" << std::dec << l.arg1 << ",";
            oss << " R" << std::dec << l.arg2 << ",";
            oss << " $" << std::hex << std::setw(4) << std::left << l.imm;
            break;

        case LC3_NOT:
            oss << " R" << std::dec << l.arg1 << ",";
            oss << " R" << std::dec << l.arg2 << ",";
            break;

        case LC3_ST:
        case LC3_STI:
            oss << " R" << std::dec << l.arg1 << ",";
            oss << " $" << std::hex << std::setw(4) << std::left << l.imm;
            break;

        case LC3_STR:
            oss << " R" << std::dec << l.arg1 << ",";
            oss << " R" << std::dec << l.arg2 << ",";
            oss << " $" << std::hex << std::setw(4) << std::left << l.imm;
            break;

        case LC3_TRAP:
            oss << " x" << l.imm;
            break;


        default:
            break;
    }
    oss << std::endl;

    return oss.str();
}
