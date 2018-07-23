/* OPCODE 
 * Opcode structures
 *
 * Stefan Wong 2018
 */

#ifndef __OPCODE_HPP
#define __OPCODE_HPP

#include <string>
#include <cstdint>

typedef struct 
{
    uint16_t    opcode;
    std::string mnemonic;
} Opcode;

#endif /*__OPCODE_HPP*/
