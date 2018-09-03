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
#include "assembler.hpp"
#include "lexer.hpp"

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
        uint16_t mem_size = 65536;
        std::string mem_filename = "data/mem_test.dat";
};

TEST_F(TestLC3, test_sentinel)
{
    unsigned int max_cycles = 70;
    std::string src_filename = "data/sentinel.asm";
    // Get a new machine
    LC3 machine;
    machine.setVerbose(false);

    Lexer lexer(machine.getOpTable(), src_filename);
    lexer.setVerbose(false);
    std::cout << "\t Lexing file " << src_filename << std::endl;
    SourceInfo src_info = lexer.lex();
    Assembler as(src_info);
    as.setVerbose(false);
    std::cout << "\t Assembling file " << src_filename << std::endl;
    as.assemble();

    // Dump the program output 
    Program asm_output = as.getProgram();
    std::cout << "\t Assembly output for file " << src_filename << std::endl;
    std::vector<Instr> as_instructions = asm_output.getInstr();
    for(unsigned int idx = 0; idx < as_instructions.size(); ++idx)
    {
        std::cout << "[" << std::dec << std::setw(4) << std::setfill('0') << idx << "]";
        std::cout << " $" << std::hex << std::setw(4) << std::setfill('0') << as_instructions[idx].adr;
        std::cout << "  " << std::hex << std::setw(4) << std::setfill('0') << as_instructions[idx].ins;
        std::cout << std::endl;
    }
    std::cout << std::endl;
    
    // Load the program into the machine
    machine.loadMemProgram(asm_output);
    machine.enable();

    unsigned int mem_offset = 0x3000;
    std::cout << "Dumping program memory from address 0x" << std::hex << std::left << mem_offset << std::endl;
    std::vector<Instr> mem_dump = machine.dumpMem(32, mem_offset);
    for(unsigned int idx = 0; idx < mem_dump.size(); ++idx)
    {
        std::cout << "["  << std::right << std::dec << std::setw(4) << std::setfill('0') << idx << "]";
        std::cout << " $" << std::hex << std::setw(4) << std::setfill('0') << mem_dump[idx].adr;
        std::cout << "  " << std::hex << std::setw(4) << std::setfill('0') << mem_dump[idx].ins;
        std::cout << std::endl;
    }


    // Execute the machine
    for(unsigned int cycle = 0; cycle < max_cycles; ++cycle)
    {
        std::cout << "Cycle " << cycle << "/" << max_cycles << "\r";
        int m_status = machine.cycle();
        if(m_status < 0)
        {
            std::cout << "machine stopped at cycle " << cycle << std::endl;
        }
    }
}


// Test the simple add program 
//TEST_F(TestLC3, test_simple_add)
//{
//    std::string add_filename = "data/add_test.asm";
//    LC3 machine;
//    machine.setVerbose(false);
//
//    Lexer lexer(machine.getOpTable(), add_filename);
//    lexer.setVerbose(false);
//    SourceInfo src_info = lexer.lex();
//    Assembler as(src_info);
//    as.setVerbose(false);
//    as.assemble();
//    
//    // Load the program into the machine
//    machine.loadMemProgram(as.getProgram());
//    unsigned int cur_cycle = 0;
//    unsigned int max_cycles = 25;
//    int machine_status;
//
//    while(cur_cycle < max_cycles)
//    {
//        machine_status = machine.runCycle();
//        if(machine_status < 0)
//            std::cerr << "Got error " << machine_status << " from LC3" << std::endl;
//        if(machine_status == 0)
//        {
//            std::cout << "Machine execution finished at cycle " << cur_cycle << std::endl;
//            break;
//        }
//        std::cout << "cycle <" << cur_cycle << ">" << std::endl;
//        cur_cycle++;
//    }
//    if(cur_cycle >= max_cycles-1)
//        std::cout << "Machine ran to max_cycles (" << max_cycles << ")" << std::endl;
//}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
