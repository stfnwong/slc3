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
#include <vector>

template <typename T> class MTrace
{
    private:
        std::vector<T> buffer;
        unsigned int trace_size;
        unsigned int trace_ptr;

    public:
        MTrace(const unsigned int size);
        ~MTrace();
        MTrace(const MTrace& that) = delete;

        // Update trace 
        void           add(const T& state);
        std::vector<T> dump(void);
        void           clear(void);
        // TODO : dump in (time) order?

        // Info
        unsigned int   getTraceSize(void) const;
        //bool           isEqual(const MTrace<T> other);

};

template <typename T> MTrace<T>::MTrace(const unsigned int size)
{
    this->trace_size = size;
    this->buffer.reserve(this->trace_size);
    this->trace_ptr = 0;
}

template <typename T> MTrace<T>::~MTrace() {} 

template <typename T> void MTrace<T>::add(const T& state)
{
    this->buffer[this->trace_ptr] = state;
    this->trace_ptr++;
    if(this->trace_ptr >= this->trace_size)
        this->trace_ptr = 0;
}

template <typename T> std::vector<T> MTrace<T>::dump(void)
{
    std::vector<T> d(this->trace_size);
    for(unsigned int i = 0; i < this->trace_size; i++)
        d[i] = this->buffer[i];

    return d;
}

template <typename T> void MTrace<T>::clear(void)
{
    this->buffer.erase(this->buffer.begin(), this->buffer.end());
    this->buffer.reserve(this->trace_size);
}

template <typename T> unsigned int MTrace<T>::getTraceSize(void) const
{
    return this->trace_size;
}


/*
 * MACHINE
 */
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
