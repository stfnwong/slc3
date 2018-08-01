/* DISASSEMBLER
 * An LC3 disassembler
 *
 * Stefan Wong 2018
 */

#ifndef __DISASSEMBLER_HPP
#define __DISASSEMBLER_HPP

#include <string>
#include "source.hpp"
#include "binary.hpp"
// Need to know something about opcodes here to
// build the mnemonic look up table 
#include "opcode.hpp"

/*
 * Disassembler
 *
 * Transforms an Program object into its correponding
 * SourceInfo object
 */ 
class Disassembler
{
    private:
        bool       verbose;
        Program    program;
        SourceInfo source;
        LineInfo   cur_line;

    private:
        // Tables for mnemonic lookup
        OpcodeTable lc3_op_table;

    private:
        // instruction disassembly
        inline bool     is_imm(const uint16_t instr) const;
        inline uint8_t  dis_op1(const uint16_t instr) const;
        inline uint8_t  dis_op2(const uint16_t instr) const;
        inline uint8_t  dis_op3(const uint16_t instr) const;
        inline uint8_t  dis_flags(const uint16_t instr) const;
        inline uint8_t  dis_opcode(const uint16_t instr) const;
        inline uint8_t  dis_of6(const uint16_t instr) const;
        inline uint16_t dis_pc9(const uint16_t instr) const;
        inline uint16_t dis_pc11(const uint16_t instr) const;
        int             disInstr(const Instr& instr);

    private:
        // TODO: mnemonic lookup. It makes more sense to have 
        // the machine model in to provide an OpTable with the 
        // correct opcode->mnemonic maps 

    public:
        Disassembler();
        ~Disassembler();

        int read(const std::string& filename);
        void disassemble(void);

        // Stats on internals 
        unsigned int numAsmInstr(void) const;
        unsigned int numSrcLines(void) const;

        void setVerbose(const bool v);
        bool getVerbose(void) const;

        SourceInfo getSourceInfo(void) const;
};

#endif /*__DISASSEMBLER_HPP*/
