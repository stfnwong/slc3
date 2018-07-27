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

// List of opcodes + mnemonic string


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
