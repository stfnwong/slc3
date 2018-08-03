/* LC3
 * Implementation of the LC3 machine 
 *
 * Stefan Wong 2018
 */

#include <iostream>
#include <iomanip>
#include <fstream>
#include "lc3.hpp"

/*
 * LC3Proc
 */
LC3Proc::LC3Proc()
{
    for(int r = 0; r < 8; r++)
        this->gpr[r] = 0;
    this->pc    = 0;
    this->mar   = 0;
    this->mdr   = 0;
    this->ir    = 0;
    this->flags = 0;
}

LC3Proc::~LC3Proc() {} 

LC3Proc::LC3Proc(const LC3Proc& that)
{
    for(int r = 0; r < 8; r++)
        this->gpr[r] = that.gpr[r];
    this->pc    = that.pc;
    this->mar   = that.mar;
    this->mdr   = that.mdr;
    this->ir    = that.ir;
    this->flags = that.flags;
}

//LC3::LC3(const uint16_t mem_size) : Machine()
LC3::LC3(const uint16_t mem_size) 
{
    this->mem_size = mem_size;
    this->allocMem();
    this->resetMem();
    this->build_op_table();
}

LC3::~LC3()
{
    delete[] this->mem;
}

void LC3::allocMem(void)
{
    this->mem = new uint16_t[this->mem_size];
}

void LC3::resetCPU(void)
{
    this->cpu.pc    = 0;
    this->cpu.flags = 0;
    for(int i = 0; i < 8; i++)
        this->cpu.gpr[i] = 0;
}

// ========  Instruction decode 
inline bool LC3::instr_is_imm(const uint16_t instr) const
{
    return (instr & 0x0020) ? true : false;
}
inline bool LC3::instr_is_jmp(const uint16_t instr) const
{
    return ((instr & 0x01C0) == 0x7) ? true : false;
}
inline uint8_t LC3::instr_get_opcode(const uint16_t instr) const
{
    return (instr & 0xF000) >> 8;
}
inline uint8_t LC3::instr_get_dest(const uint16_t instr) const
{
    return (instr & 0x0E00) >> 9;
}
inline uint8_t LC3::instr_get_baser(const uint16_t instr) const
{
    return (instr & 0x0E00) >> 9;
}
inline uint8_t LC3::instr_get_sr1(const uint16_t instr) const 
{
    return (instr & 0x01E0) >> 6;
}
inline uint8_t LC3::instr_get_sr2(const uint16_t instr) const 
{
    return (instr & 0x0007);
}
inline uint8_t LC3::instr_get_imm5(const uint16_t instr) const
{
    return (instr & 0x001F);
}
inline uint16_t LC3::instr_get_pc9(const uint16_t instr) const
{
    return (instr & 0x00FF);
}
inline uint16_t LC3::instr_get_pc11(const uint16_t instr) const
{
    return (instr & 0x03FF);
}
inline void LC3::set_flags(const uint8_t val)
{
    this->cpu.flags = (val == 0) ?
        (this->cpu.flags | LC3_FLAG_Z) : 
         this->cpu.flags;
    this->cpu.flags = (val > 0) ? 
        (this->cpu.flags | LC3_FLAG_P) : 
         this->cpu.flags;
    this->cpu.flags = (val < 0) ? 
        (this->cpu.flags | LC3_FLAG_N) : 
        this->cpu.flags;
}

/*
 * build_op_table()
 * Construct the internal opcode table.
 */
void LC3::build_op_table(void)
{
    // Build table of real ops
    for(const Opcode &op : lc3_op_list)
        this->op_table.add(op);
    // Build table of psuedo ops 
    for(const Opcode &op : lc3_psuedo_op_list)
        this->psuedo_op_table.add(op);
}

// ======== Memory 
void LC3::resetMem(void)
{
    for(uint16_t i = 0; i < this->mem_size; i++)
        this->mem[i] = 0;
    // Load the trap vector values 
    this->mem[LC3_GETC]  = LC3_TRAP20;
    this->mem[LC3_OUT]   = LC3_TRAP21;
    this->mem[LC3_PUTS]  = LC3_TRAP22;
    this->mem[LC3_IN]    = LC3_TRAP23;
    this->mem[LC3_PUTSP] = LC3_TRAP24;
    this->mem[LC3_HALT]  = LC3_TRAP25;
}

void LC3::writeMem(const uint16_t adr, const uint16_t val)
{
    this->mem[adr % this->mem_size] = val;
}

uint16_t LC3::readMem(const uint16_t adr) const
{
    return this->mem[adr % this->mem_size];
}

int LC3::loadMemFile(const std::string& filename, int offset)
{
    int status = 0;
    int num_bytes = 0;
    std::ifstream infile(filename, std::ios::binary);

    infile.seekg(0, std::ios::end);
    num_bytes = infile.tellg();
    infile.seekg(0, std::ios::beg);

    try{
        infile.read((char*) &this->mem[offset], sizeof(uint16_t) * num_bytes);
    }
    catch(std::ios_base::failure& e) {
        std::cerr << "LC3::loadMemFile caught execption [%s]" << 
            e.what() << std::endl;
        status = -1;
    }

    return status;
}

std::vector<uint16_t> LC3::dumpMem(void) const
{
    std::vector<uint16_t> mem_dump(this->mem_size);

    for(unsigned int m = 0; m < this->mem_size; m++)
        mem_dump[m] = this->mem[m];

    return mem_dump;
}

// TODO: fetch, decode, exec functions?
void LC3::fetch(void)
{
    this->cpu.mar = this->mem[this->cpu.pc];
    this->cpu.pc++;
    this->cpu.mdr = this->mem[this->cpu.mar];
    this->cpu.ir  = this->cpu.mdr;
}

void LC3::decode(void)
{
    uint8_t dst;
    uint8_t sr1, sr2;
    uint8_t imm5;
    uint16_t pc9;
    uint8_t opcode;

    opcode = this->instr_get_opcode(this->cpu.ir);
    switch(opcode)
    {
        case LC3_ADD:
            dst = this->instr_get_dest(this->cpu.ir);
            sr1 = this->instr_get_sr1(this->cpu.ir);
            if(this->instr_is_imm(this->cpu.ir))
            {
                imm5 = this->instr_get_imm5(this->cpu.ir);
                this->cpu.gpr[dst] = this->cpu.gpr[sr1] + imm5;
            }
            else
            {
                sr2 = this->instr_get_sr2(this->cpu.ir);
                this->cpu.gpr[dst] = this->cpu.gpr[sr1] + this->cpu.gpr[sr2];
            }
            this->set_flags(this->cpu.gpr[dst]);
            break;

        case LC3_AND:
            dst = this->instr_get_dest(this->cpu.ir);
            sr1 = this->instr_get_sr1(this->cpu.ir);
            if(this->instr_is_imm(this->cpu.ir))
            {
                imm5 = this->instr_get_imm5(this->cpu.ir);
                this->cpu.gpr[dst] = this->cpu.gpr[sr1] & imm5;
            }
            else
            {
                sr2 = this->instr_get_sr2(this->cpu.ir);
                this->cpu.gpr[dst] = this->cpu.gpr[sr1] & this->cpu.gpr[sr2];
            }
            this->set_flags(this->cpu.gpr[dst]);
            break;

        // Loads
        case LC3_LD:
            dst = this->instr_get_dest(this->cpu.ir);
            pc9 = this->instr_get_pc9(this->cpu.ir);
            this->cpu.gpr[dst] = this->mem[this->cpu.pc + pc9];
            this->set_flags(this->cpu.gpr[dst]);
            break;

        case LC3_NOT:
            dst = this->instr_get_dest(this->cpu.ir);
            sr1 = this->instr_get_sr1(this->cpu.ir);
            this->cpu.gpr[dst] = ~sr1;

            break;

        default:
            std::cerr << "Invalid opcode [" << std::hex << 
                std::setw(4) << std::setfill('0') << 
                opcode << std::endl;
            break;
    }
}

// Execute 
void LC3::execute(const uint16_t instr)
{

    uint16_t op;

    op = this->instr_get_opcode(instr);
    switch(op)
    {

    }
}

// Getters 
LC3Proc LC3::getProcState(void) const
{
    return this->cpu;
}

uint16_t LC3::getMemSize(void) const
{
    return this->mem_size;
}

uint8_t LC3::getFlags(void) const
{
    return this->cpu.flags;
}
bool LC3::getZero(void) const
{
    return (this->cpu.flags & LC3_FLAG_Z) ? true : false;
}
bool LC3::getPos(void) const
{
    return (this->cpu.flags & LC3_FLAG_P) ? true : false;
}
bool LC3::getNeg(void) const
{
    return (this->cpu.flags & LC3_FLAG_N) ? true : false;
}

//std::vector<Opcode> LC3::getOpcodes(void) const
//{
//    std::vector<Opcode> o(this->opcode_list);
//    return o;
//}

OpcodeTable LC3::getOpTable(void) const
{
    return this->op_table;
}
