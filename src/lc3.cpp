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

inline void LC3::set_flags(const uint8_t val)
{
    if(val == 0)
    {
        this->cpu.z = true;
        this->cpu.n = false;
        this->cpu.p = false;
    }
    else if(val > 0)
    {
        this->cpu.z = false;
        this->cpu.n = false;
        this->cpu.p = true;
    }
    else
    {
        this->cpu.z = false;
        this->cpu.n = true;
        this->cpu.p = false;
    }
}

// TODO : this will where the final op_table interface is contained
void LC3::build_op_table(void)
{
    Opcode opcode_list[] = {
        {LC3_ADD, "ADD"},
        {LC3_AND, "AND"},
        {LC3_LD,  "LD"},
        {LC3_ST,  "ST"}
    };
    // iterate over this in the C++ way
    for(const Opcode &op : opcode_list)
        this->op_table.add(op);
}

// ======== Memory 
void LC3::resetMem(void)
{
    for(uint16_t i = 0; i < this->mem_size; i++)
        this->mem[i] = 0;
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

// TODO: It may make more sense to internall deal with 
// the processor memory as an array of uint16_t
std::vector<uint16_t> LC3::dumpMem(void) const
{
    std::vector<uint16_t> mem_dump(this->mem_size);

    for(unsigned int m = 0; m < this->mem_size; m++)
        mem_dump[m] = this->mem[m];

    return mem_dump;
}

// TODO: fetch, decode, exec functions?

// Execute 
void LC3::execute(const uint16_t instr)
{
    uint8_t dst;
    uint8_t sr1, sr2;
    uint8_t imm5;
    uint16_t pc9;
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
            this->set_flags(this->cpu.gpr[dst]);
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
            this->set_flags(this->cpu.gpr[dst]);
            break;


        // Loads
        case LC3_LD:
            dst = this->instr_get_dest(instr);
            pc9 = this->instr_get_pc9(instr);
            this->cpu.gpr[dst] = this->mem[this->cpu.pc + pc9];
            this->set_flags(this->cpu.gpr[dst]);
            break;

        default:
            std::cerr << "Invalid opcode [" << std::hex << 
                std::setw(4) << std::setfill('0') << 
                op << std::endl;
            break;
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
bool LC3::getZero(void) const
{
    return this->cpu.z;
}
bool LC3::getPos(void) const
{
    return this->cpu.p;
}
bool LC3::getNeg(void) const
{
    return this->cpu.n;
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
