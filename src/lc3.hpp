/* LC3
 * Implementation of the LC3 machine 
 *
 * Stefan Wong 2018
 */

#ifndef __LC3_HPP
#define __LC3_HPP

#include <cstdint>
#include <string>
#include "machine.hpp"
#include "opcode.hpp"

// OPCODE CONSTANTS 
#define LC3_ADD     0x01
#define LC3_AND     0x05
#define LC3_BR      0x00   // branch with no flags 
#define LC3_BRP     0x00   // branch with P flag
#define LC3_BRZ     0x00   // branch with Z flag
#define LC3_BRN     0x00   // branch with N flag
#define LC3_JSR     0x04
#define LC3_LD      0x02
#define LC3_LDI     0x0A
#define LC3_LDR     0x06
#define LC3_LEA     0x0E
#define LC3_NOT     0x09
#define LC3_RTI     0x08
#define LC3_ST      0x03
#define LC3_STI     0x0B
#define LC3_STR     0x07
#define LC3_TRAP    0x0F
#define LC3_RES     0x0D
#define LC3_JMP_RET 0x0C

// LC3 psuedo ops. Format here is psuedo op followed by 
// trap table address
#define LC3_GETC    0x0020
#define LC3_OUT     0x0021
#define LC3_PUTS    0x0022
#define LC3_IN      0x0023
#define LC3_PUTSP   0x0024
#define LC3_HALT    0x0025
// Trap vector values (jump addresses)
#define LC3_TRAP20  0x0400
#define LC3_TRAP21  0x0430
#define LC3_TRAP22  0x0450
#define LC3_TRAP23  0x04A0
#define LC3_TRAP24  0x04E0
#define LC3_TRAP25  0xFD70

// LC3 Flags
#define LC3_FLAG_P  0x01
#define LC3_FLAG_Z  0x02
#define LC3_FLAG_N  0x04

// LC3 Named registers 
// NOTE : I don't think that MAR and MDR are memory mapped by default
//#define LC3_MDR
//#define LC3_MAR
#define LC3_KBSR    0xFE00
#define LC3_KBDR    0xFE02
#define LC3_DSR     0xFE04
#define LC3_DDR     0xFE06

// Memory 
#define LC3_MEM_SIZE 65535

// TODO : until the assembler/machine interface is complete,
// generate the op and psuedo op table for use with the lexer.
// Clean up this interface once the lexer internals are complete
const Opcode lc3_op_list[] = {
    {LC3_ADD,     "ADD"},
    {LC3_AND,     "AND"},
    {LC3_LD,      "LD"},
    {LC3_LDR,     "LDR"},
    {LC3_LEA,     "LEA"},
    {LC3_ST,      "ST"},
    {LC3_STI,     "STI"},
    {LC3_STR,     "STR"},
    {LC3_NOT,     "NOT"},
    {LC3_RTI,     "RTI"},
    // Jump instr 
    {LC3_JMP_RET, "JMP"},
    {LC3_JMP_RET, "RET"},
    {LC3_JSR,     "JSR"},
    {LC3_JSR,     "JSRR"},
    // BR and variants 
    {LC3_BR,      "BR"},
    {LC3_BRP,     "BRp"},
    {LC3_BRN,     "BRn"},
    // Trap vector 
    {LC3_TRAP,    "TRAP"}
};

// Psuedo ops
const Opcode lc3_psuedo_op_list[] = {
    {LC3_GETC,  "GETC"},
    {LC3_OUT,   "OUT"},
    {LC3_PUTS,  "PUTS"},
    {LC3_IN,    "IN"},
    {LC3_PUTSP, "PUTSP"},
    {LC3_HALT,  "HALT"}
};

// LC3 CPU State
class LC3Proc
{
    public:
        uint16_t gpr[8];
        uint16_t pc;
        uint16_t mar;
        uint16_t mdr;
        uint16_t ir;
        uint8_t  flags;
        
    public:
        LC3Proc();
        ~LC3Proc();
        LC3Proc(const LC3Proc& that);
        void dump(const std::string& filename);
};


//class LC3 : public Machine
//FIXME I've broken the inheritance link for the moment
//until I get the architecture sorted
class LC3 
{
    private:
        // Memory
        uint16_t* mem;
        uint32_t  mem_size;
        void      allocMem(void);
        // Processor
        LC3Proc     cpu;
        OpcodeTable op_table;
        OpcodeTable psuedo_op_table;

    private:
        // Instruction decode helper functions 
        inline bool     instr_is_imm(const uint16_t instr) const;
        inline bool     instr_is_jmp(const uint16_t instr) const;
        inline uint8_t  instr_get_opcode(const uint16_t instr) const;
        inline uint8_t  instr_get_dest(const uint16_t instr) const;
        inline uint8_t  instr_get_baser(const uint16_t instr) const;
        inline uint8_t  instr_get_sr1(const uint16_t instr) const;
        inline uint8_t  instr_get_sr2(const uint16_t instr) const;
        inline uint8_t  instr_get_imm5(const uint16_t instr) const;
        inline uint16_t instr_get_pc9(const uint16_t instr) const;
        inline uint16_t instr_get_pc11(const uint16_t instr) const;
        // Set flags 
        inline void     set_flags(const uint8_t val);
        // Build opcode table 
        void            build_op_table(void);

    private:
        // Execution cycle 
        void fetch(void);
        void decode(void);
        void eval(void);
        void operand_fetch(void);
        void exec(void);
        void store(void);

    public:
        LC3(const uint16_t mem_size);
        ~LC3();

        // Reset CPU state 
        void     resetCPU(void);
        // Memory 
        void     resetMem(void);
        void     writeMem(const uint16_t adr, const uint16_t val);
        uint16_t readMem(const uint16_t adr) const;
        int      loadMemFile(const std::string& filename, int offset);
        std::vector<uint16_t> dumpMem(void) const;
        // Execute loop
        void     execute(const uint16_t instr);

        // Getters 
        LC3Proc  getProcState(void) const;
        uint16_t getMemSize(void) const;
        uint8_t  getFlags(void) const;
        bool     getZero(void) const;
        bool     getPos(void) const;
        bool     getNeg(void) const;

        // Opcode Table (public interface)
        OpcodeTable getOpTable(void) const; //get complete table
};

#endif /*__LC3_HPP*/
