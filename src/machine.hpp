/* MACHINE
 * Generic machine object.
 * The idea of this machine object is that it can be passed to 
 * an assembler object, and emulator object, etc.
 *
 * Stefan Wong 2018
 */

#ifndef __MACHINE_HPP
#define __MACHINE_HPP

#include <cstdint>

// TODO: implement trace here?

class MProc {};


class Machine
{
    private:
        // Machine memory 
        // TODO : all memory is assumed to be flat and can therefore
        // be accessed as an array. As the machines get more complicated
        // this may need to be moved into derived classes
        uint8_t* mem;
        uint32_t mem_size;
        // Internal memory 
        void allocMem(void);

        // Machine processor

    public:
        Machine(const uint32_t mem_size);
        ~Machine();

        // Execute loop
        virtual void execute(const uint16_t instr);

        // Getters 
        uint32_t getMemSize(void) const;
};

#endif /*__MACHINE_HPP*/
