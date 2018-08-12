/* SOURCE
 * Code for source handling 
 *
 * Stefan Wong 2018
 */

#ifndef __SOURCE_HPP
#define __SOURCE_HPP

#include <string>
#include <cstdint>
#include "opcode.hpp"

#define LC3_FLAG_P 0x01
#define LC3_FLAG_Z 0x02
#define LC3_FLAG_N 0x04

typedef struct 
{
    uint16_t addr;
    std::string label;
}Symbol;

class SymbolTable
{
    private:
        std::vector<Symbol> syms;
    public:
        SymbolTable();
        ~SymbolTable();
        void add(const Symbol& s);
        void update(const unsigned int idx, const Symbol& s);
        Symbol get(const unsigned int idx) const;
        uint16_t getAddr(const std::string& label) const;
        void init(void);
        unsigned int getNumSyms(void) const;
        // debug 
        void dump(void);
};

// NOTE: This is a LC3 specific lineinfo
// structure. Consider generalizing in
// future
typedef struct{
    std::string  symbol;     //char* symbol // <- faster?;
    std::string  label;     //char* label // <- faster?;
    Opcode       opcode;
    unsigned int line_num;
    unsigned int addr;
    uint8_t      flags;
    uint16_t     arg1;
    uint16_t     arg2;
    uint16_t     arg3;
    uint16_t     imm;
    bool         is_imm;
    bool         is_label;
    bool         is_directive;
    bool         error;
} LineInfo;

/*
 * initLineInfo()
 * Reset a lineinfo struct
 */
void initLineInfo(LineInfo& l);
/*
 * compLineInfo()
 * Compare two LineInfo structs
 */
bool compLineInfo(const LineInfo& a, const LineInfo& b);

/* 
 * SourceInfo
 * Object to hold information about assembly source
 */
class SourceInfo
{
    private:
        std::vector <LineInfo> line_info;
        std::string line_to_string(const LineInfo& l);
        
    public:
        SourceInfo();
        ~SourceInfo();
        // Add/remove lines
        void add(const LineInfo& l);
        void update(const unsigned int idx, const LineInfo& l);
        LineInfo get(const unsigned int idx) const;
        unsigned int getLineNum(const unsigned int idx) const;
        unsigned int getNumLines(void) const;
        unsigned int getNumError(void) const;
        unsigned int numInstance(const uint16_t& op) const;
        unsigned int numInstance(const std::string& m) const;

        // Save/load data
        int write(const std::string& filename);
        int read(const std::string& filename);

        // String / display 
        void printLine(const unsigned int idx);
}; 

#endif /*__SOURCE_HPP*/
