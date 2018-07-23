/* ISA
 * ISA for LC-3. I'm reading this out of the book directly
 * "Introduction to Computing Systems - From bits and gates to C and beyond"
 * Yale Patt, Sanjay Patel. 2005, 2nd Edition
 *
 * Stefan Wong 2018
 */

#include <stdio.h>
#include <stdlib.h>
#include "isa.h"


PCTrace* init_pc_trace(uint32_t size)
{
    PCTrace* trace = malloc(sizeof(*trace));
    if(!trace)
    {
        fprintf(stderr, "Failed to allocate memory for trace\n");
        return NULL;
    }

    trace->trace = malloc(size * sizeof(uint32_t));
    if(!trace->trace)
    {
        fprintf(stderr, "Failed to allocate memory for trace->trace\n");
        return NULL;
    }
    trace->size = size;

    return trace;
}

void destroy_pc_trace(PCTrace* trace)
{
    free(trace->trace);
    free(trace);
}

LC3State* init_state(uint32_t mem_size, uint32_t trace_size)
{
    LC3State *state;
    state = malloc(sizeof(*state));
    if(!state)
    {
        fprintf(stderr, "Failed to allocate state memory\n");
        return NULL;
    }
    state->pc_trace = init_pc_trace(trace_size);
    if(!state->pc_trace)
        return NULL;
    state->memory = malloc(mem_size * sizeof(uint8_t));
    if(!state->memory)
    {
        fprintf(stderr, "Failed to alloc LC-3 memory\n");
        return NULL;
    }
    state->mem_size = mem_size;
    
    // Set the default values of internal registers 
    for(int i = 0; i < 8; i++)
        state->gpr[i] = 0;
    // reset program counter
    state->pc     = 0;
    // reset conditon codes
    state->cc.n   = 0;
    state->cc.z   = 0;
    state->cc.p   = 0;
    state->cc.pad = 0;

    return state;
}

void destroy_state(LC3State* state)
{
    destroy_pc_trace(state->pc_trace);
    free(state->memory);
    free(state);
}

// reset the internal memory to zero
void init_memory(LC3State* state)
{
    int i;
    for(i = 0; i < state->mem_size; i++)
        state->memory[i] = 0;
}

int load_memory(LC3State* state, const char *filename, int offset)
{
    FILE *fp;
    int num_bytes;
    uint8_t *buffer;

    fp = fopen(filename, "rb");
    if(!fp)
    {
        fprintf(stderr, "Failed to open file [%s]\n", filename);
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    num_bytes = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    buffer = &state->memory[offset];
    fread(buffer, num_bytes, 1, fp);
    fclose(fp);

    return 0;
}
