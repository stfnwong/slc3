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

class AsmProg
{
    private:
        bool verbose;
        std::vector<Instr> instructions;
        
    public:
        AsmProg();
        ~AsmProg();
        AsmProg(const AsmProg& that);

        void add(const Instr& i);
        std::vector<Instr> getInstr(void) const;
        Instr getInstr(const unsigned int idx) const;

        unsigned int getNumInstr(void) const;
        int read(const std::string& filename);
        int write(const std::string& filename);

        void setVerbose(const bool v);
        bool getVerbose(void) const;
};

#endif /*__BINARY_HPP*/
