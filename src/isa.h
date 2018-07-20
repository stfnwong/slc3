/* ISA
 * ISA for LC-3. I'm reading this out of the book directly
 * "Introduction to Computing Systems - From bits and gates to C and beyond"
 * Yale Patt, Sanjay Patel. 2005, 2nd Edition
 *
 * Stefan Wong 2018
 */

#ifndef __ISA_H
#define __ISA_H

#include <stdint.h>

// TODO : more portable to do static shift?
typedef struct 
{
    uint8_t n   : 1;
    uint8_t z   : 1; 
    uint8_t p   : 1;
    uint8_t pad : 5;
} LC3_CC;

typedef struct
{
    LC3_CC       cc;
    uint16_t     gpr[8];
    uint8_t     *memory;
    uint32_t     pc;        // what size should PC be?
} LC3State;

//// map an address to gpr index
//int adr_to_gpr(uint8_t adr);
//// map a gpr index to address
//int gpr_to_adr(uint8_t gpr);




#endif /*__ISA_H*/
