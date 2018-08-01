/* TEST_LC3
 * Test the LC3 machine object 
 *
 * Stefan Wong 2018
 */

#include <iostream>
#include <fstream>
#include <string>
#include <gtest/gtest.h>
// Modules under test 
#include "lc3.hpp"
#include "opcode.hpp"

// Fixture for testing MTrace object
class TestLC3 : public ::testing::Test
{
    protected:
        TestLC3() {}
        virtual ~TestLC3() {}
        virtual void SetUp() {}
        virtual void TearDown() {}
        bool verbose = false;       // set to true for additional output 
        // Parameters for machine under test 
        uint16_t mem_size = LC3_MEM_SIZE;
        std::string mem_filename = "data/mem_test.dat";
};

// Helper function to generate random memory contents
// Taken from https://stackoverflow.com/questions/2509679/how-to-generate-a-random-integer-number-from-within-a-range
unsigned int rand_interval(unsigned int min, unsigned int max)
{
    int r;
    const unsigned int range = 1 + max - min;
    const unsigned int buckets = RAND_MAX / range;
    const unsigned int limit = buckets * range;

    /* Create equal size buckets all in a row, then fire randomly towards
     * the buckets until you land in one of them. All buckets are equally
     * likely. If you land off the end of the line of buckets, try again. */
    do
    {
        r = rand();
    } while (r >= (int)limit);

    return min + (r / buckets);
}

// Helper function to generate sample Opcode table 
OpcodeTable test_gen_opcode_table(void)
{
    OpcodeTable op_table;

    Opcode op_list[] = {
        {0x01, "ADD"},
        {0x05, "AND"},
        {0x00, "BR" },
        {0x04, "JSR"},
        {0x02, "LD" },
        {0x06, "LDI"},
        {0x0E, "LEA"}
    };

    for(const Opcode& op : op_list)
        op_table.add(op);

    return op_table;
}

TEST_F(TestLC3, test_init)
{
    LC3 lc3(this->mem_size);
    ASSERT_EQ(this->mem_size, lc3.getMemSize());
    lc3.resetMem();
    for(uint16_t i = 0; i < this->mem_size; i++)
        ASSERT_EQ(0, lc3.readMem(i));
}

TEST_F(TestLC3, test_load_mem_file)
{
    uint16_t mem_size = 2048;
    //Machine m : mem_size(mem_size);
    LC3 lc3(mem_size);
    ASSERT_EQ(mem_size, lc3.getMemSize());

    // Clear the memory and check that all values are zeroed
    lc3.resetMem();
    for(unsigned int i = 0; i < lc3.getMemSize(); i++)
        ASSERT_EQ(0, lc3.readMem(i));

    // Generate some dummy memory contents 
    std::cout << "Generating dummy memory contents....";
    uint8_t* mem_contents = new uint8_t[mem_size];
    for(unsigned int i = 0; i < mem_size; i++)
        mem_contents[i] = rand_interval(0, 255);
    std::cout << "done" << std::endl;

    // write the contents to some file 
    std::cout << "Writing memory contents to file [" << this->mem_filename << "]...";
    std::ofstream outfile(this->mem_filename, std::ios::binary);
    if(outfile.is_open())
    {
        outfile.write((char*) mem_contents, sizeof(uint8_t) * mem_size);
        outfile.close();
    }
    else
    {
        std::cerr << "Unable to open file [" << this->mem_filename << 
            "] for writing" << std::endl;
        return;
    }
    std::cout << "done" << std::endl;

    std::cout << "Checking memory....";
    // Try to read the file contents into the Machine memory 
    int status = lc3.loadMemFile(this->mem_filename, 0);
    ASSERT_EQ(0, status);
    // Check contents
    for(unsigned int i = 0; i < mem_size; i++)
        ASSERT_EQ(mem_contents[i], lc3.readMem(i));

    std::cout << "done" << std::endl;

    delete[] mem_contents;

}

TEST_F(TestLC3, test_build_op_table)
{
    LC3 lc3(this->mem_size);
    // Opcode Table is built in constructor, read it out
    // and check against reference Opcode Table
    OpcodeTable lc3_op_table = lc3.getOpTable();

    Opcode op;
    std::cout << "Dumping opcodes from LC3 table to console..." << std::endl;
    for(unsigned int idx = 0; idx < lc3_op_table.getNumOps(); idx++)
    {
        lc3_op_table.getIdx(idx, op);
        printOpcode(op);
        std::cout << std::endl;
    }

    // Test get by opcode
    lc3_op_table.get(0x01, op);
    ASSERT_STREQ("ADD", op.mnemonic.c_str());
    lc3_op_table.get(0x05, op);
    ASSERT_STREQ("AND", op.mnemonic.c_str());
    lc3_op_table.get(0x02, op);
    ASSERT_STREQ("LD", op.mnemonic.c_str());
    lc3_op_table.get(0x03, op);
    ASSERT_STREQ("ST", op.mnemonic.c_str());

    // Test get by opcode
    lc3_op_table.get("ADD", op);
    ASSERT_EQ(0x01, op.opcode);
    lc3_op_table.get("AND", op);
    ASSERT_EQ(0x05, op.opcode);
    lc3_op_table.get("LD", op);
    ASSERT_EQ(0x02, op.opcode);
    lc3_op_table.get("ST", op);
    ASSERT_EQ(0x03, op.opcode);

    // Test the error case for string matching
    lc3_op_table.get("GARBAGE", op);
    ASSERT_EQ(0, op.opcode);
    ASSERT_STREQ("M_INVALID", op.mnemonic.c_str());

    lc3_op_table.get(0xFE, op);
    ASSERT_EQ(0, op.opcode);
    ASSERT_STREQ("M_INVALID", op.mnemonic.c_str());
}

// TODO : test arithmetic

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
