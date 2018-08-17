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
        {LC3_STI, "STI"},
        {LC3_STR, "STR"},
        {LC3_NOT, "NOT"},
        {LC3_RTI, "RTI"},
        // Jump instr 
        {LC3_JMP_RET, "JMP"},
        {LC3_JMP_RET, "RET"},
        {LC3_JSR, "JSR"},
        {LC3_JSR, "JSRR"},
        // BR and variants 
        {LC3_BR,  "BR"},
        {LC3_BRP, "BRp"},
        {LC3_BRN, "BRn"},
        {LC3_BRZ, "BRz"},
        {LC3_BRZP, "BRzp"},
        {LC3_BRNZ, "BRnz"},
        {LC3_BRNZP, "BRnzp"},
        // Trap vector 
        {LC3_TRAP, "TRAP"}
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
        std::string asm_prog_outfile = "data/test_asm_prog.dat";
        unsigned int src_length = 617; 
        OpcodeTable op_table;
        unsigned int expected_num_ops = TEST_NUM_OPS;

        SourceInfo source_info;
};

void TestAssembler::SetUp(void)
{
    this->op_table = test_build_op_table();
}

TEST_F(TestAssembler, test_init)
{
    Lexer lexer(this->op_table, this->src_filename);
    this->source_info = lexer.lex();
    Assembler as(this->source_info);
    as.setVerbose(true);

    ASSERT_EQ(0, as.getNumErr());
}

Program get_add_expected_program(void)
{
    Program prog;
    Instr instr;

    // Line 6 - .ORIG x3000
    // Line 7 - LD R1, Val1  (0x3004))
    instr.adr = 0x3000;
    instr.ins = 0x2203;
    prog.add(instr);
    // Line 8 - LD R2, Val2 (0x3005)
    instr.adr = 0x3001;
    instr.ins = 0x2403;
    prog.add(instr);
    // Line 9 - ADD R3,R1,R2
    instr.adr = 0x3002;
    instr.ins = 0x1642;
    prog.add(instr);
    // Line 10 (HALT)
    instr.adr = 0x3003;
    instr.ins = 0xF025;
    prog.add(instr);
    // Line 11 (Val1 : .FILL #1
    instr.adr = 0x3004;
    instr.ins = 0x0001;
    prog.add(instr);
    // Line 12 (Val2 : .FILL #2)
    instr.adr = 0x3005;
    instr.ins = 0x0002;
    prog.add(instr);

    return prog;
}

TEST_F(TestAssembler, test_asm_add)
{
    SourceInfo lex_output;
    std::string src_filename = "data/add_test.asm";
    Lexer lexer(this->op_table, src_filename);
    lexer.setVerbose(false);
    lex_output = lexer.lex();
    Assembler as(lex_output);
    as.setVerbose(true);
    as.setContOnError(true);

    ASSERT_EQ(0, as.getNumErr());
    as.assemble();

    // Dump the assembly log 
    std::cout << "\t Assembly log:\n\n";
    std::cout << as.getLog();
    // This test should be error free
    ASSERT_EQ(0, as.getNumErr());

    Program as_prog = as.getProgram();
    Program ex_prog = get_add_expected_program();
    std::vector<Instr> as_instructions = as_prog.getInstr();
    std::vector<Instr> ex_instructions = ex_prog.getInstr();
    std::cout << "Assembly output for program " << src_filename << std::endl;
    std::cout << " N     ADDR   DATA " << std::endl;
    for(unsigned int idx = 0; idx < ex_instructions.size(); idx++)
    {
        std::cout << "[" << std::dec << std::setw(4) << std::setfill('0') << idx << "]";
        std::cout << " $" << std::hex << std::setw(4) << std::setfill('0') << ex_instructions[idx].adr;
        std::cout << "  " << std::hex << std::setw(4) << std::setfill('0') << ex_instructions[idx].ins;
        std::cout << std::endl;
    }

    // Compare
    for(unsigned int i = 0; i < ex_instructions.size(); ++i)
    {
        std::cout << "Comparing instruction " << i+1 << "...";
        ASSERT_EQ(ex_instructions[i].adr, as_instructions[i].adr);
        ASSERT_EQ(ex_instructions[i].ins, as_instructions[i].ins);
        std::cout << std::endl;
    }
    //as_prog.save(this->asm_prog_outfile);
}

Program get_sentinel_expected_program(void)
{
    Program prog;
    Instr instr;

    // Line 5 - .ORIG x3000
    // Line 7 - LEA R1, FirstVal
    instr.adr = 0x3000;
    instr.ins = 0xE208;
    prog.add(instr);
    // Line 8 - AND R3,R3,#0
    instr.adr = 0x3001;
    instr.ins = 0x56E0;
    prog.add(instr);
    // Line 9 - LDR R4,R1,#0
    instr.adr = 0x3002;
    instr.ins = 0x6840;
    prog.add(instr);
    // Line 10 - TestEnd: BRn Done
    instr.adr = 0x3003;
    instr.ins = 0x0804;     // offset should be +4
    prog.add(instr);
    // Line 11 - ADD R3,R3,R4
    instr.adr = 0x3004;
    instr.ins = 0x16C4;
    prog.add(instr);
    // Line 12 - ADD R1,R1,#1
    instr.adr = 0x3005;
    instr.ins = 0x1261;
    prog.add(instr);
    // Line 13 - LDR R4,R1,#0
    instr.adr = 0x3006;
    instr.ins = 0x6840;
    prog.add(instr);
    // Line 14 - BRnzp TestEnd
    instr.adr = 0x3007;
    instr.ins = 0x0FFB; // offset should be -5
    prog.add(instr);

    return prog;
}

TEST_F(TestAssembler, test_asm_sentinel)
{
    SourceInfo lex_output;
    std::string src_filename = "data/sentinel.asm";
    Lexer lexer(this->op_table, src_filename);
    lexer.setVerbose(true);
    lex_output = lexer.lex();
    Assembler as(lex_output);
    as.setVerbose(true);
    as.setContOnError(true);

    ASSERT_EQ(0, as.getNumErr());
    as.assemble();

    // Assembled program
    Program as_prog = as.getProgram();
    Program ex_prog = get_sentinel_expected_program();
    std::vector<Instr> as_instructions = as.getInstrs();
    std::vector<Instr> ex_instructions = ex_prog.getInstr();

    // Assembled instruction output 
    std::cout << "\tAssembly output for program " << src_filename << std::endl;
    std::cout << " N     ADDR   DATA  <Binary>" << std::endl;
    for(unsigned int idx = 0; idx < as_instructions.size(); idx++)
    {
        std::cout << "["  << std::dec << std::setw(4) << std::setfill(' ') << idx << "]";
        std::cout << " $" << std::hex << std::setw(4) << std::setfill('0') << as_instructions[idx].adr;
        std::cout << "  " << std::hex << std::setw(4) << std::setfill('0') << as_instructions[idx].ins;
        std::cout << std::endl;
    }

    // Expected instruction output 
    std::cout << "\tExpected output for program " << src_filename << std::endl;
    std::cout << " N     ADDR   DATA  <Binary>" << std::endl;
    for(unsigned int idx = 0; idx < ex_instructions.size(); idx++)
    {
        std::cout << "["  << std::dec << std::setw(4) << std::setfill(' ') << idx << "]";
        std::cout << " $" << std::hex << std::setw(4) << std::setfill('0') << ex_instructions[idx].adr;
        std::cout << "  " << std::hex << std::setw(4) << std::setfill('0') << ex_instructions[idx].ins;
        std::cout << std::endl;
    }

    // Compare
    for(unsigned int i = 0; i < ex_instructions.size(); ++i)
    {
        std::cout << "Comparing instruction " << i << "...";
        ASSERT_EQ(ex_instructions[i].adr, as_instructions[i].adr);
        ASSERT_EQ(ex_instructions[i].ins, as_instructions[i].ins);
        std::cout << std::endl;
    }

    // Dump the assembly log 
    std::cout << "\t Assembly log:\n\n";
    std::cout << as.getLog();
}

// Test program for STRINGZ directive
Program get_stringz_test_program(void)
{
    Program prog;
    Instr instr;

    // Line 5 - .ORIG x3000
    instr.adr = 0x3010;
    instr.ins = 0x0048;
    prog.add(instr);
    instr.adr = 0x3011;
    instr.ins = 0x0065;
    prog.add(instr);
    instr.adr = 0x3012;
    instr.ins = 0x006C;
    prog.add(instr);
    instr.adr = 0x3013;
    instr.ins = 0x006C;
    prog.add(instr);
    instr.adr = 0x3014;
    instr.ins = 0x006F;
    prog.add(instr);
    instr.adr = 0x3015;
    instr.ins = 0x002C;
    prog.add(instr);
    instr.adr = 0x3016;
    instr.ins = 0x0020;
    prog.add(instr);
    instr.adr = 0x3017;
    instr.ins = 0x0057;
    prog.add(instr);
    instr.adr = 0x3018;
    instr.ins = 0x006F;
    prog.add(instr);
    instr.adr = 0x3019;
    instr.ins = 0x0072;
    prog.add(instr);
    instr.adr = 0x301A;
    instr.ins = 0x006C;
    prog.add(instr);
    instr.adr = 0x301B;
    instr.ins = 0x0064;
    prog.add(instr);
    instr.adr = 0x301C;
    instr.ins = 0x0021;
    prog.add(instr);
    //instr.adr = 0x301D;
    //instr.ins = 0x0000;   // TODO: is this a halt marker?
    //prog.add(instr);

    return prog;
}

// Test the assembly of the STRINGZ psuedo-op
TEST_F(TestAssembler, test_asm_stringz)
{
    SourceInfo lex_output;
    std::string src_filename = "data/stringz.asm";
    Lexer lexer(this->op_table, src_filename);
    lexer.setVerbose(false);
    lex_output = lexer.lex();
    Assembler as(lex_output);
    as.setVerbose(true);
    as.setContOnError(true);

    ASSERT_EQ(0, as.getNumErr());
    as.assemble();

    // Assembled program
    Program as_prog = as.getProgram();
    Program ex_prog = get_stringz_test_program();
    std::vector<Instr> as_instructions = as.getInstrs();
    std::vector<Instr> ex_instructions = ex_prog.getInstr();

    // Assembled instruction output 
    std::cout << "\tAssembly output for program " << src_filename << std::endl;
    std::cout << " N     ADDR   DATA  <Binary>" << std::endl;
    for(unsigned int idx = 0; idx < as_instructions.size(); idx++)
    {
        std::cout << "["  << std::dec << std::setw(4) << std::setfill(' ') << idx+1 << "]";
        std::cout << " $" << std::hex << std::setw(4) << std::setfill('0') << as_instructions[idx].adr;
        std::cout << "  " << std::hex << std::setw(4) << std::setfill('0') << as_instructions[idx].ins;
        std::cout << std::endl;
    }

    // Compare
    for(unsigned int i = 0; i < ex_instructions.size(); ++i)
    {
        std::cout << "Comparing instruction " << std::dec << i+1 << "...";
        ASSERT_EQ(ex_instructions[i].adr, as_instructions[i].adr);
        ASSERT_EQ(ex_instructions[i].ins, as_instructions[i].ins);
        std::cout << std::endl;
    }

    // Dump the assembly log 
    std::cout << "\t Assembly log:\n\n";
    std::cout << as.getLog();
}

// ==== Offset error test 
// Create a program that jumps to an offset which
// is too large to fit inside the opcode
SourceInfo get_offset_error_source(void)
{
    SourceInfo source;
    LineInfo line;

    // First line, load a value from a valid 
    // (in range) offset
    initLineInfo(line);
    line.addr            = 0x3000;
    line.line_num        = 1;
    line.opcode.opcode   = LC3_LD;
    line.opcode.mnemonic = "LD";
    line.arg1            = 1;
    line.imm             = 0x3050;
    source.add(line);
    // Line 2 , this one has an offset that 
    // is much too large 
    initLineInfo(line);
    line.addr            = 0x3001;
    line.line_num        = 2;
    line.opcode.opcode   = LC3_LD;
    line.opcode.mnemonic = "LD";
    line.arg1            = 1;
    line.imm             = 0xEF00;
    source.add(line);
    // Line 3, BR with large offset 
    initLineInfo(line);
    line.addr            = 0x3002;
    line.line_num        = 3;
    line.opcode.opcode   = LC3_BR;
    line.opcode.mnemonic = "BR";
    line.flags           = 0x1;  // p flag
    line.imm             = 0xEF00;
    source.add(line);

    return source;
}

// Test error handling
TEST_F(TestAssembler, test_error_handling)
{
    SourceInfo lex_output = get_offset_error_source();
    Assembler as(lex_output);
    as.setVerbose(true);
    as.setContOnError(true);

    ASSERT_EQ(0, as.getNumErr());
    as.assemble();

    // Dump the assembly log 
    std::cout << "\t Assembly log:\n\n";
    std::cout << as.getLog();
    ASSERT_EQ(2, as.getNumErr());
}

// TODO : test JMP, JSR, JSRR instructions 





// Test string formatting
TEST_F(TestAssembler, test_string_format)
{
    AsmLogEntry log_entry;
    // with error 
    log_entry.line  = 34;
    log_entry.addr  = 0x3001;
    log_entry.error = true;
    log_entry.msg   = "Test message; incorrect symbol";

    std::cout << "\tLog message with error" << std::endl;
    std::cout << log_entry.toString();

    // without error 
    log_entry.line  = 35;
    log_entry.addr  = 0x3002;
    log_entry.error = false;
    log_entry.msg   = "Test message: no error";

    std::cout << "\tLog message without error" << std::endl;
    std::cout << log_entry.toString();

    // Without message
    log_entry.line  = 35;
    log_entry.addr  = 0x3003;
    log_entry.error = false;
    log_entry.msg   = "\0";
    
    std::cout << "\tWithout message field" << std::endl;
    std::cout << log_entry.toString();
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
