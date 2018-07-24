/* TEST_LEXER 
 * Some basic test for the Lexer object 
 *
 * Stefan Wong 2018
 * */

#include <iostream>
#include <fstream>
#include <string>
#include <gtest/gtest.h>
// Modules under test 
#include "lexer.hpp"

// Fixture for testing MTrace object
class TestLexer : public ::testing::Test
{
    protected:
        TestLexer() {}
        virtual ~TestLexer() {}
        virtual void SetUp() {}
        virtual void TearDown() {}
        bool verbose = false;       // set to true for additional output 
        // Parameters for machine under test 
        uint16_t mem_size = 4096;
        std::string src_file = "data/pow10.asm";
        // Note - if the source gets modified then this value
        // needs to be updated
        unsigned int src_length = 617; 
};

TEST_F(TestLexer, test_init)
{
    Lexer l(this->src_file);
    ASSERT_EQ(this->src_length, l.getSrcLength());
    ASSERT_EQ(this->src_file, l.getFilename());

    // Also test that we can create a 'blank' lexer 
    Lexer l_blank;
    ASSERT_EQ(0, l_blank.getSrcLength());
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
