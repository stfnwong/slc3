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
#include "lc3.hpp"      // for op_table helper function

//#define DUMP_SOURCE

// Helper function to build op table for testing lexer
OpcodeTable test_build_op_table(void)
{
    OpcodeTable op_table;
    Opcode opcode_list[] = {
        {LC3_ADD, "ADD"},
        {LC3_AND, "AND"},
        {LC3_LD,  "LD"},
        {LC3_ST,  "ST"}
    };
    // iterate over this in the C++ way
    for(const Opcode &op : opcode_list)
        op_table.add(op);

    return op_table;
}

/*
 * TEST_SOURCE_INFO
 */
class TestSourceInfo : public ::testing::Test
{
    protected:
        TestSourceInfo() {}
        virtual ~TestSourceInfo() {}
        virtual void SetUp(void);
        virtual void TearDown() {}
        bool verbose = false;       // set to true for additional output 
        // Parameters for machine under test 
        uint16_t mem_size = 4096;
        std::string src_filename = "data/pow10.asm";
        unsigned int src_length = 617; 
        OpcodeTable op_table;
};

void TestSourceInfo::SetUp(void)
{
    this->op_table = test_build_op_table();
}

TEST_F(TestSourceInfo, test_init)
{
    unsigned int line_num = 0;
    SourceInfo si;

    // Read some lines from the source file 
    std::string src_text;
    std::ifstream infile(this->src_filename);
    std::string line;

    while(std::getline(infile, line))
    {
        line_num++;
        src_text += line;
        src_text.push_back('\n');
        // May as well process each line here 
        LineInfo li;
        li.line_num = line_num;
        si.add(li);
    }

    infile.close();
}

/*
 * TEST_LEXER
 */
class TestLexer : public ::testing::Test
{
    protected:
        TestLexer() {}
        virtual ~TestLexer() {}
        virtual void SetUp(void);
        virtual void TearDown() {}
        bool verbose = false;       // set to true for additional output 
        // Parameters for machine under test 
        uint16_t mem_size = 4096;
        std::string src_file = "data/pow10.asm";
        // Note - if the source gets modified then this value
        // needs to be updated
        unsigned int src_length = 617; 
        OpcodeTable op_table;
};

void TestLexer::SetUp(void)
{
    this->op_table = test_build_op_table();
}

TEST_F(TestLexer, test_init)
{
    Lexer l(this->op_table, this->src_file);
    // +1 on length is due to inserting a NULL at the end 
    ASSERT_EQ(this->src_length+1, l.getSrcLength());
    ASSERT_EQ(this->src_file, l.getFilename());

    // Also test that we can create a 'blank' lexer 
    Lexer l_blank(this->op_table);
    ASSERT_EQ(0, l_blank.getSrcLength());
}

TEST_F(TestLexer, test_lex_source)
{
    Lexer l(this->op_table, this->src_file);
    ASSERT_EQ(this->src_length+1, l.getSrcLength());
    ASSERT_EQ(this->src_file, l.getFilename());

    ASSERT_EQ(true, l.isASCII());
#ifdef DUMP_SOURCE
    for(unsigned int idx = 0; idx < this->src_length; idx++)
    {
        std::cout << l.dumpchar(idx);
    }
    std::cout << std::endl;
#endif /*DUMP_SOURCE*/

    l.lex();
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
