/* TEST_OPCODE
 *
 */

#include <iostream>
#include <fstream>
#include <string>
#include <gtest/gtest.h>
// Modules under test 
#include "opcode.hpp"

/*
 *  TEST_OPCODE 
 *  Test the Opcode table
 */
class TestOpcode : public ::testing::Test
{
    protected:
        TestOpcode() {}
        virtual ~TestOpcode() {}
        virtual void SetUp(void) {}
        virtual void TearDown(void) {}
        bool verbose = false;       // set to true for additional output 
        // Parameters for machine under test 
        uint16_t mem_size = 4096;
        std::string src_filename = "data/pow10.asm";
        unsigned int src_length = 617; 
};

TEST_F(TestOpcode, test_init)
{
    OpcodeTable op_table;
    Opcode test_op;

    // Add an instruction to table
    test_op.opcode = 0x05;
    test_op.mnemonic = "AND";
    op_table.add(test_op);
    ASSERT_EQ(1, op_table.getNumOps());
    test_op.opcode = 0x01;
    test_op.mnemonic = "ADD";
    op_table.add(test_op);
    ASSERT_EQ(2, op_table.getNumOps());
    test_op.opcode = 0x09;
    test_op.mnemonic = "NOT";
    op_table.add(test_op);
    ASSERT_EQ(3, op_table.getNumOps());

    op_table.print();

    Opcode out_op;
    op_table.get(test_op.mnemonic, out_op);
    ASSERT_STREQ(test_op.mnemonic.c_str(), out_op.mnemonic.c_str());

    // Test that we can get search opcodes by string
    Opcode search_op;           // results placed in here 
    op_table.get("AND", search_op);
    ASSERT_EQ(0x05, search_op.opcode);
    ASSERT_STREQ("AND", search_op.mnemonic.c_str());
    op_table.get("NOT", search_op);
    ASSERT_EQ(0x09, search_op.opcode);
    ASSERT_STREQ("NOT", search_op.mnemonic.c_str());
}


int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

