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
#define LC3_OFFSET_MAX 255

/*
 * AsmLogEntry
 * Logs status of a single line during the assembly process
 */
class AsmLogEntry
{
    public:
        unsigned int line;
        uint16_t     addr;
        bool         error;
        std::string  msg;

    public:
        AsmLogEntry();
        ~AsmLogEntry();
        AsmLogEntry(const AsmLogEntry& that);
        void init(void);
        std::string toString(void) const;
};

/* 
 * AsmLog
 * Assembly log object
 */
class AsmLog
{
    private:
        std::vector <AsmLogEntry> log;

    public:
        AsmLog();
        ~AsmLog();
        AsmLog(const AsmLog& that);
        // insert
        void add(const AsmLogEntry& e);
        AsmLogEntry get(const unsigned int idx) const;
        std::string getString(const unsigned int idx) const;
        std::string getString(void) const;
};

/*
 * Asssembler
 *
 * Assembles an LC3 binary from a SourceInfo structure 
 */
class Assembler
{
    private: 
        bool         verbose;
        bool         cont_on_error;
        unsigned int num_err;

    private:
        AsmLog      log;
        AsmLogEntry cur_log_entry;
        SourceInfo  src_info;
        Program     program;   // TODO: mem size later
        uint16_t    start_addr;

    private:
        // opcode part extractions
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
        void setContOnError(const bool c);
        int write(const std::string& filename);

    public:
        // Log access
        std::string getLog(void) const;
        //AsmLog      getLog(void) const;

};

#endif /*__ASSEMBLER_HPP*/
