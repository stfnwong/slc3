/* SOURCE
 * Code for source handling 
 *
 * Stefan Wong 2018
 */

#include <iostream>
#include <iomanip>
#include <sstream>
#include "source.hpp"

/*
 * SYMBOLTABLE 
 */
SymbolTable::SymbolTable() {} 
SymbolTable::~SymbolTable() {} 

void SymbolTable::add(const Symbol& s)
{
    this->syms.push_back(s);
}

void SymbolTable::update(const unsigned int idx, const Symbol& s)
{
    //if(idx > this->syms.size())
    //    return;
    this->syms[idx] = s;
}

Symbol SymbolTable::get(const unsigned int idx) const
{
    // Worth doing bounds check?
    return this->syms[idx];
}

// TODO: Currently implemented as linear search,
// change to map or something
uint16_t SymbolTable::getAddr(const std::string& s) const
{
    uint16_t addr;
    unsigned int idx;

    for(idx = 0; idx < this->syms.size(); idx++)
    {
        if(s == this->syms[idx].label)
            return this->syms[idx].addr;
    }
    addr = 0;

    return addr;
}

void SymbolTable::init(void)
{
    this->syms.clear();
}

unsigned int SymbolTable::getNumSyms(void) const
{
    return this->syms.size();
}

// TODO (debug) dump all symbols to console 
void SymbolTable::dump(void)
{
    for(unsigned int idx = 0; idx < this->syms.size(); ++idx)
    {
        Symbol s = this->syms[idx];
        std::cout << "[" << s.label << "] : 0x" << std::hex << s.addr << std::endl;
    }
}

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

/*
 * compLineInfo()
 * Compare two LineInfo objects
 */
bool compLineInfo(const LineInfo& a, const LineInfo& b)
{
    if(a.symbol != b.symbol)
        return false;
    if(a.label != b.label)
        return false;
    if(a.opcode.opcode != b.opcode.opcode)
        return false;
    if(a.opcode.mnemonic != b.opcode.mnemonic)
        return false;
    if(a.line_num != b.line_num)
        return false;
    if(a.addr != b.addr)
        return false;
    if(a.flags != b.flags)
        return false;
    if(a.arg1 != b.arg1)
        return false;
    if(a.arg2 != b.arg2)
        return false;
    if(a.arg3 != b.arg3)
        return false;
    if(a.imm != b.imm)
        return false;
    if(a.is_imm != b.is_imm)
        return false;
    if(a.is_label != b.is_label)
        return false;
    if(a.is_directive != b.is_directive)
        return false;
    if(a.error != b.error)
        return false;

    return true;
}

/*
 * SOURCEINFO 
 */
SourceInfo::SourceInfo() {} 

SourceInfo::~SourceInfo() {} 

/* 
 * line_to_string
 * Pretty-print a LineInfo struct
 */
std::string SourceInfo::line_to_string(const LineInfo& l)
{
    std::ostringstream oss;

    oss << "---------------------------------------------------------------------" << std::endl;
    oss << "Line  Type   Addr  Mnemonic    Opcode  flags   arg1  arg2  arg3  imm  " << std::endl;

    oss << std::left << std::setw(6) << std::setfill(' ') << l.line_num;
    oss << "[";
    if(l.is_imm)
        oss << "i";
    else
        oss << ".";
    if(l.is_label)
        oss << "l";
    else
        oss << ".";
    if(l.is_directive)
        oss << "d";
    else
        oss << ".";
    oss << "] ";
    oss << std::left << "0x" << std::hex << std::setw(4) << std::setfill('0') << l.addr << " ";
    oss << std::left << std::setw(12) << std::setfill(' ') << l.opcode.mnemonic;
    oss << "0x" << std::hex << std::setw(4) << std::setfill('0') << l.opcode.opcode << "   ";
    // Insert flag chars
    if(l.flags & LC3_FLAG_P)
        oss << "p";
    else
        oss << ".";
    if(l.flags & LC3_FLAG_N)
        oss << "n";
    else
        oss << ".";
    if(l.flags & LC3_FLAG_Z)
        oss << "z";
    else
        oss << ".";
    // Insert args
    oss << "  ";
    oss << " $" << std::left << std::hex << std::setw(4) << std::setfill('0') << l.arg1;
    oss << " $" << std::left << std::hex << std::setw(4) << std::setfill('0') << l.arg2;
    oss << " $" << std::left << std::hex << std::setw(4) << std::setfill('0') << l.arg3;
    oss << " $" << std::left << std::hex << std::setw(4) << std::setfill('0') << l.imm;

    // (Next line) Text 
    oss << std::endl;
    oss << "Label [" << std::setw(16) << std::setfill(' ') << l.label << "] ";
    oss << "Symbol[" << std::setw(16) << std::setfill(' ') << l.symbol << "] ";

    oss << std::endl;
    
    return oss.str();
}

void SourceInfo::add(const LineInfo& l)
{
    this->line_info.push_back(l);
}

void SourceInfo::update(const unsigned int idx, const LineInfo& l)
{
    if(idx > this->line_info.size())
        return;
    this->line_info[idx] = l;
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

// TODO : May need serialization library here
int SourceInfo::read(const std::string& filename)
{
    int status = 0;

    return status;
}

void SourceInfo::printLine(const unsigned int idx)
{
    if(idx > this->line_info.size())
        return;
    std::cout << this->line_to_string(
            this->line_info[idx]);
}
