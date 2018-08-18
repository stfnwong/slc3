/* BINARY
 * Tools for producing binaries 
 *
 * Stefan Wong 2018
 */

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include "binary.hpp"

Program::Program()
{
    this->verbose = false;
}

Program::~Program() {}


Program::Program(const Program& that)
{
    this->instructions = that.instructions;
}

// Instruction Ops 
void Program::add(const Instr& i)
{
    this->instructions.push_back(i);
}

unsigned int Program::getNumInstr(void) const
{
    return this->instructions.size();
}

std::vector<Instr> Program::getInstr(void) const
{
    return this->instructions;
}

Instr Program::getInstr(const unsigned int idx) const
{
    unsigned int num_ins = this->instructions.size();
    return this->instructions[idx % num_ins];
}

/*
 * build()
 * Generate a memory file representing the program
 *
 * TODO: Don't write ALL locations, just the ones that are
 * used and let a loader program handle the rest
 */
void Program::build(void)
{
    //Instr instr;
    //// Write the instruction list to memory 
    //for(unsigned int i = 0; i < this->instructions.size(); i++)
    //{
    //    instr = this->instructions[i];
    //    this->mem[instr.adr] = instr.ins;
    //}
}

// Memmory Ops 
void Program::writeMem(const unsigned int addr, const uint16_t val)
{
    // TODO: reformulate this is an instr and add to instruction list
    Instr instr;

    instr.adr = addr;
    instr.ins = val;
    this->instructions.push_back(instr);
}

uint16_t Program::readMem(const unsigned int addr) const
{
    // TODO : scan the instr for that addr and return the memm
    // once things are working we can eliminate the linear scan
    //return this->mem[addr % this->mem_size];
    for(unsigned int m = 0; m < this->instructions.size(); ++m)
    {
        if(addr == this->instructions[m].adr)
            return this->instructions[m].ins;
    }
    
    return 0;
}

/*
 * PROGRAM OBJECT BINARY FORMAT
 *
 * Header is just 2 bytes indicating the number of records. Each record
 * requires 4 bytes. First two bytes are address, second two bytes are
 * data for that address.
 *
 */

/* 
 * load()
 * Load a program binary from disk
 */
int Program::load(const std::string& filename)
{
    unsigned int idx;
    uint16_t num_records;
    std::ifstream infile;

    // Delete existing data 
    this->instructions.clear();

    try {
        infile.open(filename, std::ios::binary);
    }
    catch(std::ios_base::failure& e){
        std::cerr << "[" << __FUNCTION__ << "] " << e.what() << std::endl;
        return -1;
    }

    infile.read(reinterpret_cast<char*> (&num_records), sizeof(uint32_t));
    if(num_records == 0)
    {
        std::cerr << "[" << __FUNCTION__ << "] no instruction records in file " << filename << std::endl;
        infile.close();
        return -1;
    }
    // Re-allocate the internal memory to match the record size 
    //this->mem_size = (uint32_t) num_records;
    //delete[] this->mem;
    //this->alloc_mem();

    Instr instr;
    for(idx = 0; idx < num_records; idx++)
    {
        infile.read(reinterpret_cast<char*>(&instr.adr), sizeof(uint16_t));
        infile.read(reinterpret_cast<char*>(&instr.ins), sizeof(uint16_t));
        this->instructions.push_back(instr);
    }
    infile.close();

    return 0;
}

/*
 * save()
 * Save a program binary to disk
 */
int Program::save(const std::string& filename)
{
    uint32_t N;
    std::ofstream outfile;

    if(this->instructions.size() < 1)
    {
        if(this->verbose)
        {
            std::cerr << "[" << __FUNCTION__ <<
                "] no instructions in program" << std::endl;
        }
        return -1;
    }

    try {
        outfile.open(filename, std::ios::binary);
    }
    catch(std::ios_base::failure& e) {
        std::cerr << "[" << __FUNCTION__ << "] " << e.what() << std::endl;
        return -1;
    }

    N = (uint32_t) this->instructions.size();
    outfile.write(reinterpret_cast<char*>(&N), sizeof(uint32_t));
    for(unsigned int idx = 0; idx < this->instructions.size(); ++idx)
    {
        outfile.write(reinterpret_cast<char*>(
                &this->instructions[idx].adr),
                sizeof(uint16_t));
        outfile.write(reinterpret_cast<char*>(
                &this->instructions[idx].ins),
                sizeof(uint16_t));
        if(this->verbose)
        {
            std::cout << "wrote instruction " << idx + 1 << 
                "/" << this->instructions.size() << "\r";
        }
    }

    //N = (uint16_t) this->mem_size;
    //for(unsigned int idx = 0; idx < this->mem_size; idx++)
    //{
    //    outfile.write(reinterpret_cast<char*>(&idx), sizeof(uint16_t));
    //    outfile.write(reinterpret_cast<char*>(
    //            &this->mem[idx]),
    //            sizeof(uint16_t));
    //    if(this->verbose)
    //    {
    //        std::cout << "wrote instruction " << idx + 1 << 
    //            "/" << this->instructions.size() << "\r";
    //    }
    //}
    if(this->verbose)
        std::cout << std::endl;
    outfile.close();

    return 0;
}

void Program::setVerbose(const bool v)
{
    this->verbose = v;
}

bool Program::getVerbose(void) const
{
    return this->verbose;
}

void Program::print(void)
{
    std::cout << "  N   ADDR    CODE" << std::endl;
    for(unsigned int idx = 0; idx < this->instructions.size(); ++idx)
    {
        std::cout << "[" << std::dec << std::setw(4) << idx << "] " 
            << this->instr_to_string(this->instructions[idx]);
    }
}




// Transform an Instr to a string 
std::string Program::instr_to_string(const Instr& instr)
{
    std::ostringstream oss;

    oss << "$" << std::hex << std::setw(4) << std::setfill('0') 
        << instr.adr;
    oss << " 0x" << std::hex << std::setw(4) << std::setfill('0') 
        << instr.ins;
    oss << std::endl;

    return oss.str();
}
