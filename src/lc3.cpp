/* LC3
 * Implementation of the LC3 machine 
 *
 * Stefan Wong 2018
 */

#include <iostream>
#include <iomanip>
#include <fstream>
#include "lc3.hpp"

//LC3::LC3(const uint16_t mem_size) : Machine()
LC3::LC3(const uint16_t mem_size) 
{
    this->mem_size = mem_size;
    this->allocMem();
}

LC3::~LC3()
{
    delete[] this->mem;
}

void LC3::allocMem(void)
{
    this->mem = new uint8_t[this->mem_size];
}


void LC3::resetCPU(void)
{
    this->cpu.pc = 0;
    this->cpu.z  = false;
    this->cpu.n  = false;
    this->cpu.p  = false;
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



// ======== Memory 
void LC3::resetMem(void)
{
    for(uint16_t i = 0; i < this->mem_size; i++)
        this->mem[i] = 0;
}

void LC3::writeMem(const uint16_t adr, const uint8_t val)
{
    this->mem[adr % this->mem_size] = val;
}

uint8_t LC3::readMem(const uint16_t adr) const
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
        infile.read((char*) &this->mem[offset], sizeof(uint8_t) * num_bytes);
    }
    catch(std::ios_base::failure& e) {
        std::cerr << "LC3::loadMemFile caught execption [%s]" << 
            e.what() << std::endl;
        status = -1;
    }

    return status;
}


// Execute 
void LC3::execute(const uint16_t instr)
{
    uint8_t dst;
    uint8_t sr1, sr2;
    uint8_t imm5;
    uint16_t op;

    op = this->instr_get_opcode(instr);
    switch(op)
    {
        case LC3_ADD:
            dst = this->instr_get_dest(instr);
            sr1 = this->instr_get_sr1(instr);
            if(this->instr_is_imm(instr))
            {
                imm5 = this->instr_get_imm5(instr);
                this->cpu.gpr[dst] = this->cpu.gpr[sr1] + imm5;
            }
            else
            {
                sr2 = this->instr_get_sr2(instr);

                this->cpu.gpr[dst] = this->cpu.gpr[sr1] + this->cpu.gpr[sr2];
            }
            break;

        case LC3_AND:
            dst = this->instr_get_dest(instr);
            sr1 = this->instr_get_sr1(instr);
            if(this->instr_is_imm(instr))
            {
                imm5 = this->instr_get_imm5(instr);
                this->cpu.gpr[dst] = this->cpu.gpr[sr1] & imm5;
            }
            else
            {
                sr2 = this->instr_get_sr2(instr);

                this->cpu.gpr[dst] = this->cpu.gpr[sr1] & this->cpu.gpr[sr2];
            }
            break;

        default:
            std::cerr << "Invalid opcode [" << std::hex << 
                std::setw(4) << std::setfill('0') << 
                op << std::endl;
            break;

    }
}


// Getters 
uint16_t LC3::getMemSize(void) const
{
    return this->mem_size;
}
