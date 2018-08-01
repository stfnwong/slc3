/* TEST_ASSEMBLER 
 * Some basic test for the Assembler object 
 *
 * Stefan Wong 2018
 * */

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

class TestAssembler : public ::testing::Test
{
    protected:
        TestAssembler() {}
        virtual ~TestAssembler() {}
        virtual void SetUp(void);
        virtual void TearDown() {}
        bool verbose = false;       // set to true for additional output 
        // Parameters for machine under test 
        uint16_t mem_size = 4096;
        std::string src_filename = "data/pow10.asm";
        unsigned int src_length = 617; 
        OpcodeTable op_table;
        unsigned int expected_num_ops = TEST_NUM_OPS;

        SourceInfo source_info;
};

void TestAssembler::SetUp(void)
{
    this->op_table = test_build_op_table();
    ASSERT_EQ(this->expected_num_ops, this->op_table.getNumOps());
}

TEST_F(TestAssembler, test_init)
{
    Lexer lexer(this->op_table, this->src_filename);
    this->source_info = lexer.lex();
    Assembler as(this->source_info);
    as.setVerbose(true);

    ASSERT_EQ(0, as.getNumErr());
}

TEST_F(TestAssembler, test_asm_add)
{
    // TODO: narrow the scope of this unit test
    Lexer lexer(this->op_table, this->src_filename);
    lexer.setVerbose(false);
    this->source_info = lexer.lex();
    Assembler as(this->source_info);
    as.setVerbose(true);

    ASSERT_EQ(0, as.getNumErr());
    as.assemble();

    Program prog = as.getProgram();
    std::vector<Instr> instructions = as.getInstrs();
    std::cout << " N     ADDR   DATA " << std::endl;
    for(unsigned int idx = 0; idx < instructions.size(); idx++)
    {
        std::cout << "[" << std::dec << std::setw(4) << std::setfill('0') << idx << "]";
        std::cout << " $" << std::hex << std::setw(4) << std::setfill('0') << instructions[idx].adr;
        std::cout << "  " << std::hex << std::setw(4) << std::setfill('0') << instructions[idx].ins;
        std::cout << std::endl;
    }
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
        //s
}
