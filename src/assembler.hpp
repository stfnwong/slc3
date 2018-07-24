/* ASSEMBLER
 * Assembler object
 *
 * Stefan Wong 2018
 */

#ifndef __ASSEMBLER_HPP
#define __ASSEMBLER_HPP

#include <cstdint>

class Assembler
{
    private: 
        uint32_t line;
        uint8_t  num_err;

    public:
        Assembler();
        ~Assembler();

    void assemble(void);
        

};

#endif /*__ASSEMBLER_HPP*/
