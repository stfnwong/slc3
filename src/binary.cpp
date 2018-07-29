/* BINARY
 * Tools for producing binaries 
 *
 * Stefan Wong 2018
 */

#include <iostream>
#include <fstream>
#include "binary.hpp"

AsmProg::AsmProg()
{
    this->verbose = false;
}

AsmProg::~AsmProg() {} 

AsmProg::AsmProg(const AsmProg& that)
{
    this->instructions = that.instructions;
}

void AsmProg::add(const Instr& i)
{
    this->instructions.push_back(i);
}

unsigned int AsmProg::getNumInstr(void) const
{
    return this->instructions.size();
}

std::vector<Instr> AsmProg::getInstr(void) const
{
    return this->instructions;
}

Instr AsmProg::getInstr(const unsigned int idx) const
{
    unsigned int num_ins = this->instructions.size();
    return this->instructions[idx % num_ins];
}


/*
 * ASMBIN BINARY FORMAT
 *
 * Header is just 2 bytes indicating the number of records. Each record
 * requires 4 bytes. First two bytes are address, second two bytes are
 * data for that address.
 *
 */

int AsmProg::read(const std::string& filename)
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

    infile.read(reinterpret_cast<char*> (&num_records), sizeof(uint16_t));
    if(num_records == 0)
    {
        std::cerr << "[" << __FUNCTION__ << "] no instruction records in file " << filename << std::endl;
        infile.close();
        return -1;
    }

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

int AsmProg::write(const std::string& filename)
{
    uint16_t N;
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

    N = (uint16_t) this->instructions.size();
    //outfile << N;
    outfile.write(reinterpret_cast<char*>(&N), sizeof(uint16_t));
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
            std::cout << "Wrote instruction " << idx + 1 << 
                "/" << this->instructions.size() << "\r";
        }
    }
    if(this->verbose)
        std::cout << std::endl;
    outfile.close();

    return 0;
}

void AsmProg::setVerbose(const bool v)
{
    this->verbose = v;
}

bool AsmProg::getVerbose(void) const
{
    return this->verbose;
}
