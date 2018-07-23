/* TEST_MACHINE
 * Test the base machine object 
 *
 * Stefan Wong 2018
 */


#include <gtest/gtest.h>
// Modules under test 
#include "machine.hpp"

// Fixture for testing MTrace object
class TestMachine : public ::testing::Test
{
    protected:
        TestMachine() {}
        virtual ~TestMachine() {}
        virtual void SetUp() {}
        virtual void TearDown() {}
        bool verbose = false;       // set to true for additional output 
        // Parameters for machine under test 
        uint32_t mem_size = 4096;
};

// TODO : actually much of this may change
//TEST_F(TestMachine, test_init)
//{
//    Machine m(this->mem_size);
//    ASSERT_EQ(this->mem_size, m.getMemSize());
//}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
