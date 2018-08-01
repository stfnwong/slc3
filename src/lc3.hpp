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
#define LC3_ADD  0x01
#define LC3_AND  0x05
#define LC3_BR   0x00
// TODO : variations with flags get constants here for now
#define LC3_BRP  0x00
#define LC3_BRZ  0x00
#define LC3_BRN  0x00
//#define LC3_JMP  0x0C
#define LC3_JSR  0x04
#define LC3_LD   0x02
#define LC3_LDI  0x0A
#define LC3_LDR  0x06
#define LC3_LEA  0x0E
#define LC3_NOT  0x09
//#define LC3_RET  0x0C   
#define LC3_RTI  0x08
#define LC3_ST   0x03
#define LC3_STI  0x0B
#define LC3_STR  0x07
#define LC3_TRAP 0x0F
#define LC3_RES  0x0D

#define LC3_JMP_RET 0x0C

// LC3 psuedo ops. Format here is psuedo op followed by 
// trap table address
#define LC3_GETC  0x0020
#define LC3_OUT   0x0021
#define LC3_PUTS  0x0022
#define LC3_IN    0x0023
#define LC3_PUTSP 0x0024
#define LC3_HALT  0x0025

// TODO : until the assembler/machine interface is complete,
// generate the op and psuedo op table for use with the lexer.
// Clean up this interface once the lexer internals are complete
const Opcode lc3_op_list[] = {
    {LC3_ADD, "ADD"},
    {LC3_AND, "AND"},
    {LC3_LD,  "LD"},
    {LC3_LDR, "LDR"},
    {LC3_LEA, "LEA"},
    {LC3_ST,  "ST"},
    {LC3_STI, "STI"},
    {LC3_STR, "STR"},
    {LC3_NOT, "NOT"},
    {LC3_RTI, "RTI"},
    // Jump instr 
    {LC3_JMP_RET, "JMP"},
    {LC3_JMP_RET, "RET"},
    {LC3_JSR, "JSR"},
    {LC3_JSR, "JSRR"},
    // BR and variants 
    {LC3_BR,  "BR"},
    {LC3_BRP, "BRp"},
    {LC3_BRN, "BRn"},
    // Trap vector 
    {LC3_TRAP, "TRAP"}
};

const Opcode lc3_psuedo_op_list[] = {
    {LC3_GETC,  "GETC"},
    {LC3_OUT,   "OUT"},
    {LC3_PUTS,  "PUTS"},
    {LC3_IN,    "IN"},
    {LC3_PUTSP, "PUTSP"},
    {LC3_HALT,  "HALT"}
};

// LC3 CPU State
typedef struct 
{
    uint8_t  gpr[8];
    uint16_t pc;
    bool     z;
    bool     n;
    bool     p;
} LC3Proc;

//class LC3 : public Machine
//FIXME I've broken the inheritance link for the moment
//until I get the architecture sorted
class LC3 
{
    private:
        // Memory
        uint8_t* mem;
        uint16_t mem_size;
        void     allocMem(void);
        // Processor
        LC3Proc     cpu;
        OpcodeTable op_table;

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

    public:
        LC3(const uint16_t mem_size);
        ~LC3();

        // Reset CPU state 
        void     resetCPU(void);
        // Memory 
        void     resetMem(void);
        void     writeMem(const uint16_t adr, const uint8_t val);
        uint8_t  readMem(const uint16_t adr) const;
        int      loadMemFile(const std::string& filename, int offset);
        // Execute loop
        void     execute(const uint16_t instr);

        // Getters 
        LC3Proc  getProcState(void) const;
        uint16_t getMemSize(void) const;
        bool     getZero(void) const;
        bool     getPos(void) const;
        bool     getNeg(void) const;
        //std::vector<Opcode> getOpcodes(void) const;

        // Opcode Table (public interface)
        OpcodeTable getOpTable(void) const; //get complete table
};


#endif /*__LC3_HPP*/
