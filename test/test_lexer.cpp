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

//#define DUMP_SOURCE
#define TEST_NUM_OPS 10

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
        unsigned int expected_num_ops = TEST_NUM_OPS;
};

void TestSourceInfo::SetUp(void)
{
    this->op_table = test_build_op_table();
    ASSERT_EQ(this->expected_num_ops, this->op_table.getNumOps());
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
    ASSERT_EQ(this->expected_num_ops, l.opTableSize());
    // Dump the op table and show ops 
    std::cout << "Dumping lexer opcode table" << std::endl;
    l.dumpOpTable();

    l.lex();

    // Dump the source info to console
    SourceInfo lsource = l.dumpSrcInfo();
    std::cout << "Lexer created info for " << lsource.getNumLines() << " lines" << std::endl;
    for(unsigned int idx = 0; idx < lsource.getNumLines(); idx++)
    {
        LineInfo info = lsource.get(idx);
        std::cout << "==========================================" << std::endl;
        std::cout << "line      : " << std::dec << info.line_num << std::endl;
        std::cout << "addr      : " << info.addr     << std::endl;
        std::cout << "symbol    : " << info.symbol   << std::endl;
        std::cout << "label     : " << info.label    << std::endl;
        std::cout << "opcode    : " << std::hex << std::setw(4) << std::setfill('0') << info.opcode.opcode << std::endl;
        std::cout << "mnemonic  : " << info.opcode.mnemonic << std::endl;
        std::cout << "dest      : " << std::hex << std::setw(4) << std::setfill('0') << info.dest << std::endl;
        std::cout << "sr1       : " << std::hex << std::setw(4) << std::setfill('0') << info.src1 << std::endl;
        std::cout << "sr2       : " << std::hex << std::setw(4) << std::setfill('0') << info.src2 << std::endl;
        std::cout << "imm val   : " << std::hex << std::setw(4) << std::setfill('0') << info.imm  << std::endl;
        std::cout << "imm       : " << info.is_imm << std::endl;
        std::cout << "label     : " << info.is_label << std::endl;
        std::cout << "directive : " << info.is_directive << std::endl;
        std::cout << "error     : " << info.error << std::endl;
    }
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
