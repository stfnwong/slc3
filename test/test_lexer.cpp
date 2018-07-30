/* TEST_LEXER 
 * Some basic test for the Lexer object 
 *
 * Stefan Wong 2018
 * */

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <gtest/gtest.h>
// Modules under test 
#include "lexer.hpp"
#include "lc3.hpp"      // for op_table helper function
#include "source.hpp"

//#define DUMP_SOURCE
#define TEST_NUM_OPS 11

// Helper function to build op table for testing lexer
OpcodeTable test_build_op_table(void)
{
    OpcodeTable op_table;
    Opcode opcode_list[] = {
        {LC3_ADD, "ADD"},
        {LC3_AND, "AND"},
        {LC3_LD,  "LD"},
        {LC3_LDR, "LDR"},
        {LC3_LEA, "LEA"},
        {LC3_ST,  "ST"},
        {LC3_STR, "STR"},
        {LC3_NOT, "NOT"},
        // BR and variants 
        {LC3_BR,  "BR"},
        {LC3_BRP,  "BRp"},
        {LC3_BRN,  "BRn"},
    };
    // iterate over this in the C++ way
    for(const Opcode &op : opcode_list)
        op_table.add(op);

    return op_table;
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
        std::string src_file = "data/pow_10.asm";
        // Note - if the source gets modified then this value
        // needs to be updated
        unsigned int src_length = 617; 
        OpcodeTable op_table;
        unsigned int expected_num_ops = TEST_NUM_OPS;
};

void TestLexer::SetUp(void)
{
    this->op_table = test_build_op_table();
    ASSERT_EQ(this->expected_num_ops, this->op_table.getNumOps());
    std::cout << "Generated new opcode table for lexer test" << std::endl;
    this->op_table.print();
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
    ASSERT_EQ(this->expected_num_ops, this->op_table.getNumOps());
    Lexer l(this->op_table, this->src_file);

    ASSERT_EQ(true, l.isASCII());
#ifdef DUMP_SOURCE
    for(unsigned int idx = 0; idx < this->src_length; idx++)
        std::cout << l.dumpchar(idx);
    std::cout << std::endl;
#endif /*DUMP_SOURCE*/
    ASSERT_EQ(this->expected_num_ops, l.opTableSize());
    // Dump the op table and show ops 
    std::cout << "Dumping lexer opcode table" << std::endl;
    l.dumpOpTable();
    l.setVerbose(true);

    SourceInfo lsource;
    lsource = l.lex();

    // Dump the source info to console
    std::cout << "Lexer created info for " << lsource.getNumLines() << " lines" << std::endl;
    for(unsigned int idx = 0; idx < lsource.getNumLines(); idx++)
    {
        LineInfo info = lsource.get(idx);
        printLineInfo(info);
    }

    // TODO: test that this is correct - need a known good sourceinfo to compare against
    unsigned int num_ops;
    std::cout << "Opcodes by frequency: " << std::endl;
    for(unsigned int n = 0; n < this->op_table.getNumOps(); n++)
    {
        Opcode op;
        this->op_table.get(n, op);
        num_ops = lsource.numInstance(op.opcode);
        std::cout << "[" << std::hex << std::setw(4) << op.opcode 
            << "] (" << op.mnemonic << ") - " << num_ops << std::endl;
    }
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
