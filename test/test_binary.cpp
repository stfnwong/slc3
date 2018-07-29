/* TEST_BINARY
 * Test the objects for dealing with LC3 binaries
 *
 * Stefan Wong 2018
 */
#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <gtest/gtest.h>
// Modules under test 
#include "assembler.hpp"
#include "lexer.hpp"
#include "lc3.hpp"      // for op_table helper function
#include "source.hpp"
#include "binary.hpp"

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


class TestBinary : public ::testing::Test
{
    protected:
        TestBinary() {}
        virtual ~TestBinary() {}
        virtual void SetUp(void);
        virtual void TearDown(void) {}
        bool verbose = false;       // set to true for additional output 
        // Parameters for machine under test 
        uint16_t mem_size = 4096;
        std::string asm_src_filename = "data/pow10.asm";
        std::string bin_output_filename = "data/test_asm_output.bin";
        unsigned int src_length = 617; 
        OpcodeTable op_table;
        unsigned int expected_num_ops = TEST_NUM_OPS;
        SourceInfo source_info;
};

void TestBinary::SetUp(void)
{
    this->op_table = test_build_op_table();
    ASSERT_EQ(this->expected_num_ops, this->op_table.getNumOps());
}

// Basic init test 
TEST_F(TestBinary, test_init)
{
    AsmBin asm_bin;
    ASSERT_EQ(0, asm_bin.getNumInstr());
}

TEST_F(TestBinary, test_write)
{
    // Prepare an AsmBin object
    Lexer lexer(this->op_table, this->asm_src_filename);
    lexer.setVerbose(false);
    this->source_info = lexer.lex();
    Assembler as(this->source_info);
    as.setVerbose(false);
    as.assemble();

    // Write binary to disk
    int status;
    AsmBin prog = as.getProgram();
    prog.setVerbose(true);
    status = prog.write(this->bin_output_filename);
    ASSERT_EQ(0, status);
}

TEST_F(TestBinary, test_read)
{
    // Prepare an AsmBin object
    Lexer lexer(this->op_table, this->asm_src_filename);
    lexer.setVerbose(false);
    this->source_info = lexer.lex();
    Assembler as(this->source_info);
    as.setVerbose(false);
    as.assemble();

    // Write binary to disk
    int status;
    AsmBin prog = as.getProgram();
    prog.setVerbose(true);
    status = prog.write(this->bin_output_filename);
    ASSERT_EQ(0, status);

    // Read into new object, compare 
    // object members 
    AsmBin read_prog;
    read_prog.setVerbose(true);
    read_prog.read(this->bin_output_filename);

    ASSERT_EQ(prog.getNumInstr(), read_prog.getNumInstr());
    std::vector<Instr> instr_write;
    std::vector<Instr> instr_read;

    instr_write = prog.getInstr();
    instr_read  = read_prog.getInstr();
    ASSERT_EQ(instr_write.size(), instr_read.size());

    for(unsigned idx = 0; idx < instr_write.size(); ++idx)
    {
        ASSERT_EQ(instr_write[idx].adr, instr_read[idx].adr);
        ASSERT_EQ(instr_write[idx].ins, instr_read[idx].ins);
    }
}


int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
