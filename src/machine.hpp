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

    public:
        Machine();
        ~Machine();

        // Execute loop
        virtual void execute(const uint16_t instr) = 0;

};

#endif /*__MACHINE_HPP*/
