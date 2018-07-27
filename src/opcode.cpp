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
    fprintf(stdout, "%s\t: $%04X", o.mnemonic.c_str(), o.opcode);
}

OpcodeTable::OpcodeTable() {} 

OpcodeTable::~OpcodeTable() {} 

void OpcodeTable::add(const Opcode& o)
{
    this->op_list.push_back(o);
}

void OpcodeTable::get(const std::string& mnemonic, Opcode& o) const
{
    // TODO : something faster than linear search
    // LC3 is small enough to get away with this but may as well
    // come back and do it properly once things are working
    unsigned int idx;

    o.opcode = 0;
    o.mnemonic = "M_INVALID";
    for(idx = 0; idx < this->op_list.size(); idx++)
    {
        if(mnemonic == this->op_list[idx].mnemonic)
            o = this->op_list[idx];
    }
}

void OpcodeTable::get(const uint16_t opcode, Opcode& o) const
{
    // TODO : same as above but 'search by opcode'
    unsigned int idx;

    o.opcode = 0;
    o.mnemonic = "M_INVALID";
    for(idx = 0; idx < this->op_list.size(); idx++)
    {
        if(opcode == this->op_list[idx].opcode)
            o = this->op_list[idx];
    }
}

void OpcodeTable::getIdx(const unsigned int idx, Opcode& o) const
{
    if(idx < this->op_list.size())
        o = this->op_list[idx];
}

void OpcodeTable::print(void) const
{

}

unsigned int OpcodeTable::getNumOps(void) const
{
    return this->op_list.size();
}
