/* LC3
 * Implementation of the LC3 machine 
 *
 * Stefan Wong 2018
 */

#ifndef __LC3_HPP
#define __LC3_HPP

#include "machine.hpp"


struct LC3Proc
{
    uint16_t pc;
};

class LC3 : public Machine
{
    private:

    public:
        LC3();
        ~LC3();

};


#endif /*__LC3_HPP*/
