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

inline uint8_t Assembler::asm_in8(const uint16_t arg)
{
    return 0x0000 | (arg & 0x00FF);
}

inline uint16_t Assembler::asm_pc9(const uint16_t arg)
{
    return 0x0000 | (arg & 0x01FF);
}

inline uint16_t Assembler::asm_pc11(const uint16_t arg)
{
    return 0x0000 | (arg & 0x07FF);
}

// Instruction assembly
void Assembler::asm_add(const LineInfo& line)
{
    Instr instr;

    instr.ins = 0;
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
    instr.adr = line.addr;

    this->program.add(instr);
}

void Assembler::asm_and(const LineInfo& line)
{
    Instr instr;

    instr.ins = 0;
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
    instr.adr = line.addr;

    this->program.add(instr);
}

void Assembler::asm_br(const LineInfo& line)
{
    Instr instr;

    instr.ins = 0;
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

void Assembler::asm_jmp(const LineInfo& line)
{
    Instr instr;

    instr.ins = 0;
    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] (src line " 
            << line.line_num << ") assembling JMP" << std::endl;
    }
    instr.ins = (instr.ins | (line.opcode.opcode << 12));
    instr.ins = (instr.ins | this->asm_arg2(instr.ins));
    instr.adr = line.addr;

    this->program.add(instr);
}

void Assembler::asm_jsr(const LineInfo& line)
{
    Instr instr;

    instr.ins = 0;
    // Is this JSR or JSRR?
    if(line.arg1 > 0)       // JSR 
    {
        if(this->verbose)
        {
            std::cout << "[" << __FUNCTION__ << "] (src line " 
                << line.line_num << ") assembling JSR" << std::endl;
        }
        instr.ins = (instr.ins | this->asm_pc11(instr.ins));
    }
    else                    // JSRR
    {
        if(this->verbose)
        {
            std::cout << "[" << __FUNCTION__ << "] (src line " 
                << line.line_num << ") assembling JSRR" << std::endl;
        }
        instr.ins = (instr.ins | this->asm_arg2(instr.ins));
    }
    instr.ins = (instr.ins | (line.opcode.opcode << 12));
    instr.adr = line.addr;

    this->program.add(instr);

}

void Assembler::asm_lea(const LineInfo& line)
{
    Instr instr;

    instr.ins = 0;
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

void Assembler::asm_ld(const LineInfo& line)
{
    Instr instr;

    instr.ins = 0;
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

    instr.ins = 0;
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

void Assembler::asm_not(const LineInfo& line)
{
    Instr instr;

    instr.ins = 0;
    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] (src line " 
            << line.line_num << ") assembling NOT" << std::endl;
    }
    instr.ins = (instr.ins | (line.opcode.opcode << 12));
    instr.ins = (instr.ins | this->asm_arg1(line.arg1));
    instr.ins = (instr.ins | this->asm_arg2(line.imm));
    // NOT Has lower 6 bits set to 1
    instr.ins = (instr.ins | 0x001F);
    instr.adr = line.addr;

    this->program.add(instr);
}

void Assembler::asm_st(const LineInfo& line)
{
    Instr instr;

    instr.ins = 0;
    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] (src line " 
            << line.line_num << ") assembling ST" << std::endl;
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

    instr.ins = 0;
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

void Assembler::asm_sti(const LineInfo& line)
{
    Instr instr;

    instr.ins = 0;
    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] (src line " 
            << line.line_num << ") assembling STI" << std::endl;
    }
    instr.ins = (instr.ins | (line.opcode.opcode << 12));
    instr.ins = (instr.ins | this->asm_arg1(line.arg1));
    instr.ins = (instr.ins | this->asm_arg2(line.arg2));
    instr.ins = (instr.ins | this->asm_of6(line.imm));
    instr.adr = line.addr;

    this->program.add(instr);
}

void Assembler::asm_trap(const LineInfo& line)
{
    Instr instr;

    instr.ins = 0;
    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] (src line " 
            << line.line_num << ") assembling TRAP" << std::endl;
    }
    instr.ins = (instr.ins | (line.opcode.opcode << 12));
    instr.ins = (instr.ins | this->asm_in8(instr.ins));
    instr.adr = line.addr;

    this->program.add(instr);
}

// ================ DIRECTIVES / PSUEDO OPS 
void Assembler::dir_blkw(const LineInfo& line)
{
    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] (src line " <<
            line.line_num << ") assembling .BLKW" << std::endl;
    }

    unsigned int addr;
    for(addr = line.addr; addr < line.addr + line.imm; addr++)
        this->program.writeMem(addr, 0xFFFF); // TODO: proper value
}

void Assembler::dir_fill(const LineInfo& line)
{
    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] (src line " <<
            line.line_num << ") assembling .FILL" << std::endl;
    }
    // Next location in memory gets line.imm
    this->program.writeMem(line.addr, line.imm);
}

void Assembler::dir_orig(const LineInfo& line)
{
    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] (src line " <<
            line.line_num << ") assembling .ORIG" << std::endl;
    }
    this->start_addr = line.imm;
}

void Assembler::dir_stringz(const LineInfo& line)
{
    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] (src line " <<
            line.line_num << ") assembling .STRINGZ" << std::endl;
        std::cout << "NOT YET IMPLEMENTED" << std::endl;
    }
}

/*
 * assemble()
 * Assemble the program into a memory image 
 */
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
                    cur_line.opcode.mnemonic << std::endl;
            }
            // TODO: some kind of indirection that allows for 
            // flexibility (jump table)?
            // TODO : Change lexer so that Directives are mnemonic
            if(cur_line.opcode.mnemonic == ".BLKW")
                this->dir_blkw(cur_line);
            else if(cur_line.opcode.mnemonic == ".FILL")
                this->dir_fill(cur_line);
            else if(cur_line.opcode.mnemonic == ".ORIG")
                this->dir_orig(cur_line);
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
            case LC3_JSR:
                this->asm_jsr(cur_line);
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
                break;
            case LC3_TRAP:
                this->asm_trap(cur_line);
                break;
            default:
                std::cout << "[" << __FUNCTION__ <<
                    "] (line " << cur_line.line_num << 
                    ") invalid opcode 0x" << std::hex << 
                    std::setw(2) << std::setfill('0') <<
                    cur_line.opcode.opcode << " with mnemonic " <<
                    std::uppercase << cur_line.opcode.mnemonic << 
                    std::endl;
                break;
        }
    }
}

unsigned int Assembler::getNumErr(void) const
{
    return this->num_err;
}

Program Assembler::getProgram(void) const
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

int Assembler::write(const std::string& filename)
{
    return this->program.save(filename);
}
