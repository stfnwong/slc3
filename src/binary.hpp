/* BINARY
 * Tools for producing binaries 
 *
 * Stefan Wong 2018
 */

#ifndef __BINARY_HPP
#define __BINARY_HPP

#include <cstdint>
#include <string>
#include <vector>

// A single instruction
typedef struct 
{
    uint16_t adr;
    uint16_t ins;
} Instr;

/*
 * Program
 * Holds an LC3 assembly program
 */
class Program
{
    private:
        bool verbose;
        std::vector<Instr> instructions;

    public:
        Program();
        ~Program();
        Program(const Program& that);

        // Instruction ops 
        void               add(const Instr& i);
        std::vector<Instr> getInstr(void) const;
        Instr              getInstr(const unsigned int idx) const;
        unsigned int       getNumInstr(void) const;
        void               build(void);
        // Memory ops 
        void               writeMem(const unsigned int addr, const uint16_t val);
        uint16_t           readMem(const unsigned int addr) const;
        // Save/load 
        int                load(const std::string& filename);
        int                save(const std::string& filename);
        // Console output
        void               setVerbose(const bool v);
        bool               getVerbose(void) const;
};

#endif /*__BINARY_HPP*/
