/* ASSEMBLER
 * Assembler object
 *
 * Stefan Wong 2018
 */

#ifndef __ASSEMBLER_HPP
#define __ASSEMBLER_HPP

#include <cstdint>
#include <vector>
#include "source.hpp"

class Assembler
{
    private: 
        bool verbose;
        unsigned int num_err;

    private:
        SourceInfo src_info;
        std::vector<uint16_t> instrs;   // could be POD array later

    private:
        inline uint16_t asm_arg1(const uint16_t arg);
        inline uint16_t asm_arg2(const uint16_t arg);
        inline uint16_t asm_arg3(const uint16_t arg);
        // Assembly of instructions 
        void asm_add(const LineInfo& line);


    public:
        Assembler(const SourceInfo& si);
        ~Assembler();

        void assemble(void);
        unsigned int getNumErr(void) const;
        std::vector<uint16_t> getInstrs(void) const;
        
        void setVerbose(const bool v);
        bool getVerbose(void) const;
};

#endif /*__ASSEMBLER_HPP*/
