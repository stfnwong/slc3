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

/*
 * MTrace
 * Machine trace wrapper
 */
template <typename T> class MTrace
{
    private:
        std::vector<T> buffer;
        unsigned int trace_size;
        unsigned int trace_ptr;

    public:
        MTrace(const unsigned int size);
        ~MTrace();
        MTrace(const MTrace<T>& that);

        // Update trace 
        void           add(const T& state);
        std::vector<T> dump(void);
        std::vector<T> dumpOrdered(void);
        T              get(const unsigned int idx) const;
        void           clear(void);

        // Info
        unsigned int   getTraceSize(void) const;
        bool           isEqual(const MTrace<T>& that); // TODO: re-write as operator overload
};


// Ctor
template <typename T> MTrace<T>::MTrace(const unsigned int size)
{
    this->trace_size = size;
    this->buffer.reserve(this->trace_size);
    this->trace_ptr = 0;
}

// Dtor 
template <typename T> MTrace<T>::~MTrace() {} 

// Copy ctor 
template <typename T> MTrace<T>::MTrace(const MTrace<T>& that)
{
    this->trace_size = that.trace_size;
    this->trace_ptr  = that.trace_ptr;
    // Copy the trace 
    this->buffer.reserve(this->trace_size);
    for(unsigned t = 0; t < this->buffer.size(); ++t)
        this->buffer[t] = that.buffer[t];
}

/*
 * add()
 * Insert a new machine state into the trace
 */
template <typename T> void MTrace<T>::add(const T& state)
{
    this->buffer[this->trace_ptr] = state;
    this->trace_ptr++;
    if(this->trace_ptr >= this->trace_size)
        this->trace_ptr = 0;
}

/* 
 * dump()
 * Return a vector containing the contents of the machine 
 * trace buffer.
 */
template <typename T> std::vector<T> MTrace<T>::dump(void)
{
    std::vector<T> d(this->trace_size);
    for(unsigned int i = 0; i < this->trace_size; i++)
        d[i] = this->buffer[i];

    return d;
}

/*
 * dumpOrdered()
 * Dump the trace such that the first elemen of the output vector
 * is the most recent state, the next element is the next most 
 * recent state, and so on.
 */
template <typename T> std::vector<T> MTrace<T>::dumpOrdered(void)
{
    int cur_pos;
    std::vector<T> d(this->trace_size);

    cur_pos = this->trace_ptr;
    for(unsigned int t = 0; t < this->buffer.size(); ++t)
    {
        d[t] = this->buffer[cur_pos];
        cur_pos--;
        if(cur_pos < 0)
            cur_pos = this->buffer.size()-1;
    }

    return d;
}

/*
 * get()
 * Return the machine trace object at the specified index. Note that
 * the index will be wrapped to modulo buffer_size.
 */
template <typename T> T MTrace<T>::get(const unsigned idx) const
{
    //return this->buffer[idx % this->buffer.size()];
    return this->buffer[idx];
}

/*
 * clear()
 * Erase the contents of the machine trace buffer
 */
template <typename T> void MTrace<T>::clear(void)
{
    this->buffer.erase(this->buffer.begin(), this->buffer.end());
    this->buffer.reserve(this->trace_size);
}

/*
 * getTraceSize()
 * Get the size of the trace buffer. Note that this returns the maximum
 * size, rather than the current size.
 */
template <typename T> unsigned int MTrace<T>::getTraceSize(void) const
{
    return this->trace_size;
}

/*
 * isEqual()
 * Test if this MTrace object is equal to another MTrace object
 */
template <typename T> bool MTrace<T>::isEqual(const MTrace<T>& that)
{
    if(this->trace_size != that.trace_size)
        return false;
    for(unsigned int t = 0; t < this->buffer.size(); ++t)
    {
        if(this->buffer[t] != that.buffer[t])
            return false;
    }
    
    return true;
}


/*
 * MACHINE
 * Generic machine object
 */
class Machine
{
    private:

    public:
        Machine();
        ~Machine();

        // Execute loop
        virtual void reset(void) = 0;
        virtual void cycle(const uint16_t instr) = 0;

};

#endif /*__MACHINE_HPP*/
