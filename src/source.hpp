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

void initLineInfo(LineInfo& l);
void printLineInfo(const LineInfo& l);

class SourceInfo
{
    private:
        std::vector <LineInfo> line_info;
        
    public:
        SourceInfo();
        ~SourceInfo();
        // Add/remove lines
        void add(const LineInfo& l);
        LineInfo get(const unsigned int idx) const;
        unsigned int getLineNum(const unsigned int idx) const;
        unsigned int getNumLines(void) const;
        unsigned int getNumError(void) const;
        unsigned int numInstance(const uint16_t& op) const;
        unsigned int numInstance(const std::string& m) const;

        // Save/load data
        int write(const std::string& filename);
        int read(const std::string& filename);
}; 


#endif /*__SOURCE_HPP*/
