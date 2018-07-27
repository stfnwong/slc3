/* OPCODE 
 * Opcode structures
 *
 * Stefan Wong 2018
 */

#ifndef __OPCODE_HPP
#define __OPCODE_HPP

#include <string>
#include <vector>
#include <cstdint>

typedef struct 
{
    uint16_t    opcode;
    std::string mnemonic;
} Opcode;

void printOpcode(const Opcode& o);

class OpcodeTable
{
    private:
        std::vector<Opcode> op_list;
        
    public:
        OpcodeTable();
        ~OpcodeTable();
        void add(const Opcode& o);
        void get(const std::string& mnemonic, Opcode& o) const;
        void get(const uint16_t opcode, Opcode& o) const;
        void getIdx(const unsigned int idx, Opcode& o) const;

        // TODO: create toString() method
        void print(void) const;

        // generic getters 
        unsigned int getNumOps(void) const;

};

#endif /*__OPCODE_HPP*/
