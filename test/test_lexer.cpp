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

// Helper function to generate the correct SourceInfo for 
// the file "data/add_test.asm"
SourceInfo get_add_test_source_info(void)
{
    SourceInfo info;
    LineInfo line;

    // Line 6 (.ORIG x3000)
    initLineInfo(line);
    line.line_num        = 6;
    line.addr            = 0x3000;
    line.opcode.mnemonic = ".ORIG";
    line.opcode.opcode   = 0;
    line.imm             = 0x3000;
    line.is_directive    = true;
    info.add(line);
    // Line 7 (LD, R1, Val1)
    initLineInfo(line);
    line.line_num        = 7;
    line.addr            = 0x3001;
    line.opcode.mnemonic = "LD";
    line.opcode.opcode   = 0x02;
    line.arg1            = 0x01;
    line.imm             = 0x3005;
    line.symbol          = "Val1";
    info.add(line);
    // Line 8 (LD,R2,Val2)
    initLineInfo(line);
    line.line_num        = 8;
    line.addr            = 0x3002;
    line.opcode.mnemonic = "LD";
    line.opcode.opcode   = 0x02;
    line.arg1            = 0x02;
    line.imm             = 0x3006;
    line.symbol          = "Val2";
    info.add(line);
    // Line 9 (ADD R3,R1,R2)
    initLineInfo(line);
    line.line_num        = 9;
    line.addr            = 0x3003;
    line.opcode.mnemonic = "ADD";
    line.opcode.opcode   = 0x01;
    line.arg1            = 0x03;
    line.arg2            = 0x01;
    line.arg3            = 0x02;
    info.add(line);
    // Line 10 (HALT)
    initLineInfo(line);
    line.line_num        = 10;
    line.addr            = 0x3004;
    line.opcode.opcode   = 0x0F;
    line.opcode.mnemonic = "TRAP";
    line.imm             = 0x25;
    info.add(line);
    // Line 11 (Val1 .FILL #1)
    initLineInfo(line);
    line.line_num        = 11;
    line.addr            = 0x3005;
    line.opcode.opcode   = 0x0;
    line.opcode.mnemonic = ".FILL";
    line.label           = "Val1";
    line.imm             = 1;
    line.is_label        = true;
    line.is_directive    = true;
    info.add(line);
    // Line 12 (Val2 .FILL #2)
    initLineInfo(line);
    line.line_num        = 12;
    line.addr            = 0x3006;
    line.opcode.opcode   = 0x0;
    line.opcode.mnemonic = ".FILL";
    line.label           = "Val2";
    line.imm             = 2;
    line.is_label        = true;
    line.is_directive    = true;
    info.add(line);
    // Line 12 (.END)
    initLineInfo(line);
    line.line_num        = 13;
    line.addr            = 0x3007;
    line.opcode.opcode   = 0x0;
    line.opcode.mnemonic = ".END";
    line.is_directive    = true;
    info.add(line);

    return info;

}

TEST_F(TestLexer, test_lex_add)
{
    std::string asm_src_filename = "data/add_test.asm";
    Lexer lexer(this->op_table, asm_src_filename);
    lexer.setVerbose(true);
    SourceInfo lsource = lexer.lex();

    // Dump the source info
    std::cout << "[" << __FUNCTION__ << "] Lexer created info for " << lsource.getNumLines() << " lines" << std::endl;
    for(unsigned int idx = 0; idx < lsource.getNumLines(); idx++)
        lsource.printLine(idx);

    // Dump the symbol table 
    SymbolTable sym_table = lexer.dumpSymTable();
    std::cout << "[" << __FUNCTION__ << "] Dumping symbol table" << std::endl;
    for(unsigned int s = 0; s < sym_table.getNumSyms(); ++s)
    {
        Symbol sym = sym_table.get(s);
        std::cout << std::left << std::setw(12) << std::setfill(' ') << 
            sym.label << " : 0x" << std::hex << std::setfill('0') << sym.addr << std::endl;
    }

    SourceInfo expected_info = get_add_test_source_info();
    std::cout << "[" << __FUNCTION__ << "] expected source info" << std::endl;
    for(unsigned int idx = 0; idx < lsource.getNumLines(); idx++)
        expected_info.printLine(idx);

    // TODO : Because I am not yet dealing with the .END directive we skip the last line
    std::cout << "[" << __FUNCTION__ << "] checking source info" << std::endl;
    for(unsigned int idx = 0; idx < expected_info.getNumLines(); ++idx)
    {
        LineInfo lex_line = lsource.get(idx);
        LineInfo exp_line = expected_info.get(idx);
        std::cout << "Checking line " << idx+1 << "(source line " << std::dec << lex_line.line_num << ") ...";
        ASSERT_EQ(true, compLineInfo(lex_line, exp_line));
        std::cout << " done" << std::endl;
    }
}

TEST_F(TestLexer, test_lex_pow)
{
    //ASSERT_EQ(this->expected_num_ops, this->op_table.getNumOps());
    SourceInfo lsource;
    Lexer l(this->op_table);
    l.setVerbose(true);
    l.loadFile(this->src_file);

    // Lex the source file
    lsource = l.lex();

    // Dump the source info to console
    std::cout << "[" << __FUNCTION__ << "] Lexer created info for " << lsource.getNumLines() << " lines" << std::endl;
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

    std::cout << "[" << __FUNCTION__ << "] Dumping symbol table" << std::endl;
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
