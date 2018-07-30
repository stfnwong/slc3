/* SOURCE
 * Code for source handling 
 *
 * Stefan Wong 2018
 */

#include <iostream>
#include <iomanip>
#include "source.hpp"

// LineInfo
void initLineInfo(LineInfo& l)
{
    l.line_num = 0;
    l.addr     = 0;
    l.symbol   = "\0";
    l.label    = "\0";
    l.opcode   = {0x0, "DEFAULT"},
    l.arg1     = 0;
    l.arg2     = 0;
    l.arg3     = 0;
    l.imm      = 0;
    l.flags    = 0;
    l.is_imm   = false;
    l.is_label = false;
    l.error    = false;
    l.is_directive = false;
}

void printLineInfo(const LineInfo& l)
{
        std::cout << "==========================================" << std::endl;
        std::cout << "line      : " << std::dec << l.line_num << std::endl;
        std::cout << "addr      : " << l.addr     << std::endl;
        std::cout << "symbol    : " << l.symbol   << std::endl;
        std::cout << "label     : " << l.label    << std::endl;
        std::cout << "opcode    : " << std::hex << std::setw(4) << std::setfill('0') << l.opcode.opcode << std::endl;
        //std::cout << "flags     : " << std::hex << std::setw(2) << l.flags << std::endl;
        std::cout << "flags     : ";
        if(l.flags & LC3_FLAG_P)
            std::cout << "p";
        if(l.flags & LC3_FLAG_N)
            std::cout << "n";
        if(l.flags & LC3_FLAG_Z)
            std::cout << "z";
        std::cout << std::endl;
        std::cout << "mnemonic  : " << l.opcode.mnemonic << std::endl;
        std::cout << "arg1      : " << std::hex << std::setw(4) << std::setfill('0') << l.arg1 << std::endl;
        std::cout << "arg2      : " << std::hex << std::setw(4) << std::setfill('0') << l.arg2 << std::endl;
        std::cout << "arg3      : " << std::hex << std::setw(4) << std::setfill('0') << l.arg3 << std::endl;
        std::cout << "imm val   : " << std::hex << std::setw(4) << std::setfill('0') << l.imm  << std::endl;
        std::cout << "imm       : " << l.is_imm << std::endl;
        std::cout << "label     : " << l.is_label << std::endl;
        std::cout << "directive : " << l.is_directive << std::endl;
        std::cout << "error     : " << l.error << std::endl;
}

/*
 * SOURCEINFO 
 */
SourceInfo::SourceInfo() {} 

SourceInfo::~SourceInfo() {} 

void SourceInfo::add(const LineInfo& l)
{
    this->line_info.push_back(l);
}

LineInfo SourceInfo::get(const unsigned int idx) const
{
    if(idx < this->line_info.size())
        return this->line_info[idx];
    else
    {
        LineInfo l;
        l.line_num = 0;
        
        return l;
    }
}

unsigned int SourceInfo::getLineNum(const unsigned int idx) const
{
    return this->line_info[idx].line_num;
}

unsigned int SourceInfo::getNumLines(void) const
{
    return this->line_info.size();
}

unsigned int SourceInfo::getNumError(void) const
{
    unsigned int num_err = 0;
    
    for(unsigned int idx = 0; idx < this->line_info.size(); idx++)
    {
        if(this->line_info[idx].error)
            num_err++;
    }

    return num_err;
}

/*
 * numInstance() 
 * 
 * Returns the number of times a particular opcode appears in
 * the source l.
 */
unsigned int SourceInfo::numInstance(const uint16_t& op) const
{
    unsigned int n = 0;
    unsigned int idx;

    for(idx = 0; idx < this->line_info.size(); idx++)
    {
        if(this->line_info[idx].opcode.opcode == op)
            n++;
    }

    return n;
}

/*
 * numInstance() 
 * 
 * Returns the number of times a particular mnemonic appears in
 * the source info.
 */
unsigned int SourceInfo::numInstance(const std::string& m) const
{
    unsigned int n = 0;
    unsigned int idx;

    for(idx = 0; idx < this->line_info.size(); idx++)
    {
        if(this->line_info[idx].opcode.mnemonic == m)
            n++;
    }

    return n;
}


// Save/load data 
// TODO : may need a serialization library here 
int SourceInfo::write(const std::string& filename)
{
    int status = 0;
    //uint32_t num_records, sym_len, label_len;

    //num_records = this->line_info.size();

    return status;
}

int SourceInfo::read(const std::string& filename)
{
    int status = 0;

    return status;
}



