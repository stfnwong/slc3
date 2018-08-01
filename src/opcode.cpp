/* OPCODE 
 * Opcode structures
 *
 * Stefan Wong 2018
 */

#include <cctype>
#include <cstdio>
#include "opcode.hpp"

// Print an opcode 
void printOpcode(const Opcode& o)
{
    //fprintf(stdout, "%s\t: $%04X", o.mnemonic.c_str(), o.opcode);
}

OpcodeTable::OpcodeTable() {} 

OpcodeTable::~OpcodeTable() {} 

void OpcodeTable::add(const Opcode& o)
{
    this->op_list.push_back(o);
}

// TODO : something faster than linear search
// LC3 is small enough to get away with this but may as well
// come back and do it properly once things are working
void OpcodeTable::get(const std::string& mnemonic, Opcode& o) const
{
    unsigned int idx;

    for(idx = 0; idx < this->op_list.size(); idx++)
    {
        if(mnemonic == this->op_list[idx].mnemonic)
        {
            o.opcode   = this->op_list[idx].opcode;
            o.mnemonic = this->op_list[idx].mnemonic;
            return;
            //o = this->op_list[idx];
        }
    }
    o.opcode = 0;
    o.mnemonic = "M_INVALID";
}

void OpcodeTable::get(const uint16_t opcode, Opcode& o) const
{
    // TODO : same as above but 'search by opcode'
    unsigned int idx;

    for(idx = 0; idx < this->op_list.size(); idx++)
    {
        if(opcode == this->op_list[idx].opcode)
        {
            //o = this->op_list[idx];
            o.opcode   = this->op_list[idx].opcode;
            o.mnemonic = this->op_list[idx].mnemonic;
            return;
        }
    }
    o.opcode = 0;
    o.mnemonic = "M_INVALID";
}

std::string OpcodeTable::getMnemonic(const uint16_t opcode) const
{
    // At some point maybe I should deal with all these linear scans..
    for(unsigned int idx = 0; idx < this->op_list.size(); ++idx)
    {
        if(opcode == this->op_list[idx].opcode)
            return this->op_list[idx].mnemonic;
    }

    return "OP_INVAILD";
}

void OpcodeTable::getIdx(const unsigned int idx, Opcode& o) const
{
    if(idx < this->op_list.size())
        o = this->op_list[idx];
}

void OpcodeTable::init(void)
{
    this->op_list.clear();
}

// Print the entire contents of the OpcodeTable to console
void OpcodeTable::print(void) const
{
    unsigned int idx;
    
    fprintf(stdout, "%ld instructions in table\n", this->op_list.size());
    for(idx = 0; idx < this->op_list.size(); idx++)
    {
        fprintf(stdout, "op %04d : [%s] $%04X\n", 
                idx+1, 
                this->op_list[idx].mnemonic.c_str(), 
                this->op_list[idx].opcode);
    }
}

unsigned int OpcodeTable::getNumOps(void) const
{
    return this->op_list.size();
}
