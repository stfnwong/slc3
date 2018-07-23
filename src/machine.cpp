/* MACHINE
 * Generic machine object 
 *
 * Stefan Wong 2018
 */

#include "machine.hpp"


Machine::Machine(const uint32_t mem_size)
{
    this->mem_size = mem_size;
    this->allocMem();
}

Machine::~Machine()
{
    delete[] this->mem;
}

void Machine::allocMem(void)
{
    this->mem = new uint8_t[this->mem_size];
}


// Getters
uint32_t Machine::getMemSize(void) const
{
    return this->mem_size;
}
