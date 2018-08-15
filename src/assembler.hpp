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
#include "binary.hpp"

//TODO : This also needs to move to some machine specific 
// place for generic (visitor) implementation
#define LC3_ADR_SIZE 65535

/*
 * Asssembler
 *
 * Assembles an LC3 binary from a SourceInfo structure 
 */
class Assembler
{
    private: 
        bool verbose;
        unsigned int num_err;

    private:
        SourceInfo  src_info;
        Program     program;   // TODO: mem size later
        uint16_t    start_addr;

    private:
        inline uint16_t asm_arg1(const uint16_t arg);
        inline uint16_t asm_arg2(const uint16_t arg);
        inline uint16_t asm_arg3(const uint16_t arg);
        inline uint16_t asm_imm5(const uint16_t arg);
        inline uint16_t asm_of6(const uint16_t arg);
        inline uint8_t  asm_in8(const uint16_t arg);
        inline uint16_t asm_pc9(const uint16_t arg);
        inline uint16_t asm_pc11(const uint16_t arg);
        // Assembly of instructions 
        void asm_add(const LineInfo& line);
        void asm_and(const LineInfo& line);
        void asm_br(const LineInfo& line);
        void asm_jmp(const LineInfo& line);
        void asm_jsr(const LineInfo& line);
        void asm_lea(const LineInfo& line);
        void asm_ld(const LineInfo& line);
        void asm_ldr(const LineInfo& line);
        void asm_not(const LineInfo& line);
        void asm_ret(const LineInfo& line);
        void asm_rti(const LineInfo& line);
        void asm_st(const LineInfo& line);
        void asm_sti(const LineInfo& line);
        void asm_str(const LineInfo& line);
        void asm_trap(const LineInfo& line);

    private:
        // Handle directives / psuedo ops 
        void dir_blkw(const LineInfo& line);
        void dir_fill(const LineInfo& line);
        void dir_orig(const LineInfo& line);
        void dir_stringz(const LineInfo& line);

    public:
        Assembler(const SourceInfo& si);
        ~Assembler();

        void assemble(void);
        unsigned int getNumErr(void) const;
        Program getProgram(void) const;
        std::vector<Instr> getInstrs(void) const;
        
        void setVerbose(const bool v);
        bool getVerbose(void) const;

        int write(const std::string& filename);

};

#endif /*__ASSEMBLER_HPP*/
