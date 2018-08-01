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

// TODO: move this to util function?
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
        std::string src_file = "data/pow10.asm";
        // Note - if the source gets modified then this value
        // needs to be updated
        unsigned int src_length = 617; 
        OpcodeTable op_table;
        //unsigned int expected_num_ops = TEST_NUM_OPS;
};

void TestLexer::SetUp(void)
{
    this->op_table = test_build_op_table();
    std::cout << "Generated new opcode table for lexer test containing"
        << op_table.getNumOps() << " opcodes" << std::endl;
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
#ifdef DUMP_SOURCE
    for(unsigned int idx = 0; idx < this->src_length; idx++)
        std::cout << l.dumpchar(idx);
    std::cout << std::endl;
#endif /*DUMP_SOURCE*/
    // Dump the op table and show ops 
    std::cout << "Dumping lexer opcode table" << std::endl;
    l.dumpOpTable();
}

TEST_F(TestLexer, test_lex_source)
{
    //ASSERT_EQ(this->expected_num_ops, this->op_table.getNumOps());
    SourceInfo lsource;
    Lexer l(this->op_table);
    l.setVerbose(true);
    l.loadFile(this->src_file);

    // Lex the source file
    lsource = l.lex();

    // Dump the source info to console
    std::cout << "Lexer created info for " << lsource.getNumLines() << " lines" << std::endl;
    for(unsigned int idx = 0; idx < lsource.getNumLines(); idx++)
        lsource.printLine(idx);

    // TODO: test that this is correct - need a known good sourceinfo to compare against
    // May need to make this by hand...
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

    // Also dump the symbol table 
    SymbolTable sym_table = l.dumpSymTable();

    std::cout << "Dumping symbol table" << std::endl;
    for(unsigned int s = 0; s < sym_table.getNumSyms(); ++s)
    {
        Symbol sym = sym_table.get(s);
        std::cout << std::left << std::setw(12) << std::setfill(' ') << 
            sym.label << " : 0x" << std::hex << std::setfill('0') << sym.addr << std::endl;
    }
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
