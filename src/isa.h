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
    uint32_t *trace;
    uint32_t  size;
} PCTrace;

typedef struct
{
    LC3_CC       cc;
    uint16_t     gpr[8];
    uint8_t     *memory;
    uint32_t     mem_size;
    uint32_t     pc;        // what size should PC be?
    PCTrace     *pc_trace;
} LC3State;

PCTrace* init_pc_trace(uint32_t size);
void destroy_pc_trace(PCTrace* trace);
LC3State* init_state(uint32_t mem_size, uint32_t trace_size);
void destroy_state(LC3State* state);

// Memory functions
// TODO : is this really an ISA function?
void init_memory(LC3State* state);
int load_memory(LC3State* state, const char *filename, int offset);


#endif /*__ISA_H*/
