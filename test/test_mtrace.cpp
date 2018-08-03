/* TEST_LC3
 * Test the LC3 machine object 
 *
 * Stefan Wong 2018
 */

#include <iostream>
#include <vector>
#include <gtest/gtest.h>
// Modules under test 
#include "machine.hpp"
#include "lc3.hpp"      // For the LC3Proc object

// Fixture for testing MTrace object
class TestMTrace : public ::testing::Test
{
    protected:
        TestMTrace() {}
        virtual ~TestMTrace() {}
        virtual void SetUp() {}
        virtual void TearDown() {}
        bool verbose = false;       // set to true for additional output 
        // Parameters for machine under test 
        unsigned int trace_size = 16;
};

// Helper function to compare two LC3Proc's for equality
bool lc3proc_equal(const LC3Proc& p1, const LC3Proc& p2)
{
    if(p1.pc != p2.pc)
        return false;
    if(p1.flags != p2.flags)
        return false;
    if(p1.mar != p2.mar)
        return false;
    if(p1.mdr != p2.mdr)
        return false;
    if(p1.ir != p2.ir)
        return false;
    for(int g = 0; g < 8; g++)
    {
        if(p1.gpr[g] != p2.gpr[g])
            return false;
    }

    return true;
}

TEST_F(TestMTrace, test_init)
{
    MTrace<LC3Proc> trace(this->trace_size);
    ASSERT_EQ(this->trace_size, trace.getTraceSize());
}

TEST_F(TestMTrace, test_add)
{
    MTrace<LC3Proc> trace(this->trace_size);
    ASSERT_EQ(this->trace_size, trace.getTraceSize());

    // Create some dummy trace objects 
    std::vector <LC3Proc> test_trace(this->trace_size + 1);

    for(unsigned int t = 0; t < this->trace_size+1; t++)
    {
        LC3Proc p;
        p.pc = t;
        for(int g = 0; g < 8; g++)
            p.gpr[g] = g + t;
        p.flags = 0;
        test_trace[t] = p;
    }

    // First test the trace object with just trace_size procs
    for(unsigned int t = 0; t < this->trace_size; t++)
        trace.add(test_trace[t]);

    std::vector<LC3Proc> trace_dump = trace.dump();
    for(unsigned int t = 0; t < trace_dump.size(); t++)
        ASSERT_EQ(true, lc3proc_equal(trace_dump[t], test_trace[t]));

    // Now try adding more data so that the trace 'wraps'
    trace.add(test_trace[this->trace_size + 1]);
    trace_dump = trace.dump();
    ASSERT_EQ(true, lc3proc_equal(trace_dump[0], test_trace[this->trace_size+1]));
    ASSERT_EQ(this->trace_size, trace.getTraceSize());
    // Offset the PC by 256
    for(unsigned int t = 0; t < test_trace.size(); t++)
        test_trace[t].pc += 256;
    for(unsigned int t = 0; t < test_trace.size(); t++)
        trace.add(test_trace[t]);

    //trace_dump = trace.dump();
    //ASSERT_EQ(true, lc3proc_equal(trace_dump[0], test_trace[1]));
    //for(unsigned int t = 1; t < test_trace.size(); t++)
    //    ASSERT_EQ(true, lc3proc_equal(test_trace[t], trace_dump[t]));

}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
