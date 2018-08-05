/* LC3
 * Implementation of the LC3 machine 
 *
 * Stefan Wong 2018
 */

#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include "lc3.hpp"

/*
 * LC3Proc
 * Processor state object for LC3
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

void LC3Proc::diff(const LC3Proc& that)
{
    for(int r = 0; r < 8; r++)
    {
        if(this->gpr[r] != that.gpr[r])
        {
            std::cout << "[" << __FUNCTION__ << "] register " 
                << r << std::endl;
        }
    }
    if(this->pc != that.pc)
    {
        std::cout << "[" << __FUNCTION__ << "] this->pc  <0x" << 
            std::hex << std::setw(4) << this->pc << "> != that.pc <0x" <<
            std::hex << std::setw(4) << that.pc << std::endl;
    }
    if(this->mar != that.mar)
    {
        std::cout << "[" << __FUNCTION__ << "] this->mar <0x" << 
            std::hex << std::setw(4) << this->mar << "> != that.mar <0x" <<
            std::hex << std::setw(4) << that.mar << std::endl;
    }
    if(this->mdr != that.mdr)
    {
        std::cout << "[" << __FUNCTION__ << "] this->mdr <0x" << 
            std::hex << std::setw(4) << this->mdr << "> != that.mdr <0x" <<
            std::hex << std::setw(4) << that.mdr << std::endl;
    }
    if(this->ir != that.ir)
    {
        std::cout << "[" << __FUNCTION__ << "] this->ir  <0x" << 
            std::hex << std::setw(4) << this->ir << "> != that.ir <0x" <<
            std::hex << std::setw(4) << that.ir << std::endl;
    }
    if(this->flags != that.flags)
    {
        std::cout << "[" << __FUNCTION__ << "] this->flags <0x" << 
            std::hex << std::setw(4) << this->flags << "> != that.flags <0x" <<
            std::hex << std::setw(4) << that.flags << std::endl;
    }
    if(this->sr1 != that.sr1)
    {
        std::cout << "[" << __FUNCTION__ << "] this->sr1 <0x" << 
            std::hex << std::setw(4) << this->sr1 << "> != that.sr1 <0x" <<
            std::hex << std::setw(4) << that.sr1 << std::endl;
    }
    if(this->sr2 != that.sr2)
    {
        std::cout << "[" << __FUNCTION__ << "] this->sr2 <0x" << 
            std::hex << std::setw(4) << this->sr2 << "> != that.sr2 <0x" <<
            std::hex << std::setw(4) << that.sr2 << std::endl;
    }
    if(this->dst != that.dst)
    {
        std::cout << "[" << __FUNCTION__ << "] this->dst <0x" << 
            std::hex << std::setw(4) << this->dst << "> != that.dst <0x" <<
            std::hex << std::setw(4) << that.dst << std::endl;
    }
}

std::string LC3Proc::toString(void) const
{
    std::ostringstream oss;
    // Processor state 
    oss << "pc :   ir :   flags: mar    mdr    sr1    sr2    dst   op" << std::endl;
    oss << "0x" << std::hex << std::setw(4) << std::setfill('0') << this->pc << " ";
    oss << "0x" << std::hex << std::setw(4) << std::setfill('0') << this->ir << " ";
    // Flags 
    oss << "[";
    if(this->flags & LC3_FLAG_N)
        oss << "n";
    else 
        oss << ".";
    if(this->flags & LC3_FLAG_Z)
        oss << "z";
    else 
        oss << ".";
    if(this->flags & LC3_FLAG_P)
        oss << "p";
    else 
        oss << ".";
    oss << "]  ";

    oss << "0x" << std::hex << std::setw(4) << std::setfill('0') << this->mar << " ";
    oss << "0x" << std::hex << std::setw(4) << std::setfill('0') << this->mdr << " ";
    oss << "0x" << std::hex << std::setw(4) << std::setfill('0') << this->sr1 << " ";
    oss << "0x" << std::hex << std::setw(4) << std::setfill('0') << this->sr2 << " ";
    oss << "0x" << std::hex << std::setw(4) << std::setfill('0') << this->dst << " ";
    oss << "0x" << std::hex << std::setw(4) << std::setfill('0') << this->cur_opcode << " ";
    // Next line 
    oss << std::endl;
    // Register contents 
    oss << "r[0]   r[1]   r[2]   r[3]   r[4]   r[5]   r[6]   r[7]" << std::endl;
    for(int r = 0; r < 8; ++r)
        oss << "0x" << std::hex << std::setw(4) << std::setfill('0') << this->gpr[r] << " ";
    // End
    oss << std::endl << std::endl;

    return oss.str();
}

/*
 * LC3 
 * Constructor for and LC3 object
 */
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
    this->state.pc    = 0;
    this->state.flags = 0;
    for(int i = 0; i < 8; i++)
        this->state.gpr[i] = 0;
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
inline uint8_t LC3::instr_get_of6(const uint16_t instr) const
{
    return (instr & 0x003F);
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
    this->state.flags = (val == 0) ?
        (this->state.flags | LC3_FLAG_Z) : 
         this->state.flags;
    this->state.flags = (val > 0) ? 
        (this->state.flags | LC3_FLAG_P) : 
         this->state.flags;
    this->state.flags = (val < 0) ? 
        (this->state.flags | LC3_FLAG_N) : 
        this->state.flags;
}

/*
 * sign extension functions 
 */
inline uint16_t LC3::sext5(const uint8_t v) const
{
    return (v & 0x0010) ? (v | 0xFFE0) : v;
}
inline uint16_t LC3::sext6(const uint8_t v) const
{
    return (v & 0x0020) ? (v | 0xFFC0) : v;
}
inline uint16_t LC3::sext9(const uint16_t v) const
{
    return (v & 0x0100) ? (v | 0xFE00) : v;
}
inline uint16_t LC3::sext11(const uint16_t v) const
{
    return (v & 0x0400) ? (v | 0xF800) : v;
}
inline uint16_t LC3::zext8(const uint8_t v) const
{
    return (0x0000 | v);
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
        std::cerr << "[" << __FUNCTION__ << "] caught execption [%s]" << 
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

void LC3::fetch(void)
{
    if(this->verbose)
        std::cout << "[" << __FUNCTION__ << "] FETCHing next instruction" << std::endl; 

    //this->state.mar = this->mem[this->state.pc];
    this->state.pc++;
    this->state.mdr = this->mem[this->state.mar];
    this->state.ir  = this->state.mdr;
}

void LC3::decode(void)
{
    this->state.cur_opcode = this->instr_get_opcode(this->state.ir);

    // Setup register values for this opcode
    switch(this->state.cur_opcode)
    {
        case LC3_ADD:
            this->state.dst = this->instr_get_dest(this->state.ir);
            this->state.sr1 = this->instr_get_sr1(this->state.ir);
            if(this->instr_is_imm(this->state.ir))
                this->state.imm = this->sext5(this->instr_get_imm5(this->state.ir));
            else
                this->state.sr2 = this->instr_get_sr2(this->state.ir);

            break;

        case LC3_AND:
            this->state.dst = this->instr_get_dest(this->state.ir);
            this->state.sr1 = this->instr_get_sr1(this->state.ir);
            if(this->instr_is_imm(this->state.ir))
                this->state.imm  = this->sext5(this->instr_get_imm5(this->state.ir));
            else
                this->state.sr2 = this->instr_get_sr2(this->state.ir);

            break;

        case LC3_LEA:
            this->state.dst = this->instr_get_dest(this->state.ir);
            this->state.imm = this->sext9(this->instr_get_pc9(this->state.ir));
            this->state.sr1 = this->instr_get_sr1(this->state.ir);
            break;

        case LC3_LD:
            this->state.dst = this->instr_get_dest(this->state.ir);
            this->state.imm = this->sext9(this->instr_get_pc9(this->state.ir));
            break;

        case LC3_LDI:
            this->state.dst = this->instr_get_dest(this->state.ir);
            this->state.imm = this->sext9(this->instr_get_pc9(this->state.ir));
            break;

        case LC3_LDR:
            this->state.dst = this->instr_get_dest(this->state.ir);
            this->state.sr1 = this->instr_get_sr1(this->state.ir); //base
            this->state.imm = this->sext6(this->instr_get_of6(this->state.ir));
            break;
            
        case LC3_NOT:
            this->state.dst = this->instr_get_dest(this->state.ir);
            this->state.sr1 = this->instr_get_sr1(this->state.ir);
            break;

        case LC3_ST:
            this->state.mar = this->state.imm + this->state.pc;
            break;

        case LC3_STI:
            this->state.mar = this->state.imm + this->state.pc;
            break;

        case LC3_STR:
            this->state.mar = this->state.imm + this->state.gpr[this->state.sr1];
            break;

        default:
            std::cout << "[" << __FUNCTION__ << "] invalid opcode <0x"
                << std::hex << std::setw(4) << std::setfill('0') 
                << this->state.cur_opcode << std::endl;
            break;
    }
}

void LC3::eval_addr(void)
{
    switch(this->state.cur_opcode)
    {
        // Instructions that have no EVAL_ADDR actions
        case LC3_ADD:
        case LC3_AND:
            break;

        case LC3_LD:
            this->state.mar = this->state.imm + this->state.pc;
            break;

        case LC3_LDI:
            this->state.mar = this->state.imm + this->state.pc;
            break;

        case LC3_LDR:
            // DR, BaseR, of6
            this->state.mar = this->state.imm + this->state.gpr[this->state.sr1];
            break;

        case LC3_NOT:       // No EVAL_ADDR actions
            break;

        case LC3_ST:
            this->state.mar = this->state.imm + this->state.pc;
            break;

        case LC3_STI:
            this->state.mar = this->state.imm + this->state.pc;
            break;

        case LC3_STR:
            // DR, BaseR, of6
            this->state.mar = this->state.imm + this->state.gpr[this->state.sr1];
            break;

        default:
            std::cout << "[" << __FUNCTION__ << "] invalid opcode <0x"
                << std::hex << std::setw(4) << std::setfill('0')
                << this->state.cur_opcode << std::endl;
            break;
    }
    
}

// Execute 
void LC3::execute(void)
{
    // Execute the instruction
    switch(this->state.cur_opcode)
    {
        case LC3_ADD:
            if(this->instr_is_imm(this->state.ir))
                this->state.gpr[this->state.dst] = this->state.gpr[this->state.sr1] + this->state.imm;
            else
                this->state.gpr[this->state.dst] = this->state.gpr[this->state.sr1] + this->state.gpr[this->state.sr2];
            break;

        case LC3_AND:
            if(this->instr_is_imm(this->state.ir))
                this->state.gpr[this->state.dst] = this->state.gpr[this->state.sr1] & this->state.imm;
            else
                this->state.gpr[this->state.dst] = this->state.gpr[this->state.sr1] & this->state.gpr[this->state.sr2];
            break;

        case LC3_LEA:
            break;


        // Instructions that have no EXECUTE phase 
        case LC3_LD:
        case LC3_LDI:
        case LC3_LDR:
            break;

        case LC3_NOT:
            this->state.gpr[this->state.dst] = ~this->state.sr1;
            break;

        case LC3_ST:
            break;

        default:
            std::cerr << "Invalid opcode [" << std::hex << 
                std::setw(4) << std::setfill('0') << 
                this->state.cur_opcode << std::endl;
            break;
    }
    // Set flags 
    this->set_flags(this->state.gpr[this->state.dst]);
}

void LC3::store(void)
{
    switch(this->state.cur_opcode)
    {
        // Instructions that have no STORE phase 
        case LC3_ADD:
        case LC3_AND:
        case LC3_BR:
        case LC3_JMP_RET:
        case LC3_JSR:
            break;

        case LC3_LEA:
            break;

        case LC3_LD:
            this->state.gpr[this->state.dst] = this->mem[this->state.pc + this->state.imm];
            break;

        case LC3_LDI:
            //this->state.gpr[this->state.dst] = this->mem[this->state.imm];
            this->state.gpr[this->state.dst] = this->mem[this->state.mar];
            break;

        case LC3_LDR:
            this->state.gpr[this->state.dst] = this->mem[this->state.sr1 + this->state.imm];
            break;

        case LC3_NOT:
            break;

        case LC3_RTI:
            break;
            
        case LC3_ST:
            this->mem[this->state.imm] = this->state.gpr[this->state.sr1];
            break;

        case LC3_STI:
            this->mem[this->state.mar] = this->state.gpr[this->state.sr1];
            break;

        case LC3_STR:
            this->mem[this->state.mar] = this->state.gpr[this->state.sr1];
            break;


        default:
            std::cerr << "Invalid opcode [" << std::hex << 
                std::setw(4) << std::setfill('0') << 
                this->state.cur_opcode << std::endl;
    }
}

// Run an instruction cycle 
int LC3::runCycle(void)
{
    int status = 0;     // TODO: how to set incorrect status?

    while(1)
    {
        this->fetch();
        this->decode();
        this->eval_addr();
        //this->operand_fetch();
        this->execute();
        this->store();
        // TODO: check execution errors?
    }

    return status;
}

// Getters 
LC3Proc LC3::getProcState(void) const
{
    return this->state;
}

uint16_t LC3::getMemSize(void) const
{
    return this->mem_size;
}

uint8_t LC3::getFlags(void) const
{
    return this->state.flags;
}
bool LC3::getZero(void) const
{
    return (this->state.flags & LC3_FLAG_Z) ? true : false;
}
bool LC3::getPos(void) const
{
    return (this->state.flags & LC3_FLAG_P) ? true : false;
}
bool LC3::getNeg(void) const
{
    return (this->state.flags & LC3_FLAG_N) ? true : false;
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

// Verbose 
void LC3::setVerbose(const bool v)
{
    this->verbose = v;
}

bool LC3::getVerbose(void) const
{
    return this->verbose;
}
