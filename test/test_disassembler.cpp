/* TEST_DISASSEMBLER 
 * Some basic test for the Disassembler object 
 *
 * Stefan Wong 2018
 * */

#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <gtest/gtest.h>
// Modules under test 
#include "disassembler.hpp"
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

class TestDisassembler : public ::testing::Test
{
    protected:
        TestDisassembler() {}
        virtual ~TestDisassembler() {}
        virtual void SetUp(void);
        virtual void TearDown() {}
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

void TestDisassembler::SetUp(void)
{
    this->op_table = test_build_op_table();
}

TEST_F(TestDisassembler, test_init)
{
    Disassembler dis;
    ASSERT_EQ(0, dis.numAsmInstr());
    ASSERT_EQ(0, dis.numSrcLines());
}



// Expected disassembly for sentinel program
// Note that assembler directives are removedc
SourceInfo get_sentinel_test_source_info(void)
{
    SourceInfo info;
    LineInfo line;

    // Line 7 (LD, R1, Val1)
    initLineInfo(line);
    line.line_num        = 7;
    line.addr            = 0x3000;
    line.opcode.mnemonic = "LEA";
    line.opcode.opcode   = 0xE;
    line.arg1            = 0x01;
    line.imm             = 0x3009;
    line.symbol          = "FirstVal";
    info.add(line);
    // Line 8 (AND R3,R3,#0)
    initLineInfo(line);
    line.line_num        = 8;
    line.addr            = 0x3001;
    line.opcode.mnemonic = "AND";
    line.opcode.opcode   = 0x05;
    line.arg1            = 3;
    line.arg2            = 3;
    line.imm             = 0;
    line.is_imm          = true;
    info.add(line);
    // Line 9 (LDR R4,R1,#0)
    initLineInfo(line);
    line.line_num        = 9;
    line.addr            = 0x3002;
    line.opcode.mnemonic = "LDR";
    line.opcode.opcode   = 0x06;
    line.arg1            = 4;
    line.arg2            = 1;
    line.imm             = 0;
    line.is_imm          = true;
    info.add(line);
    // Line 10 (TestEnd)
    initLineInfo(line);
    line.line_num        = 10;
    line.addr            = 0x3003;
    line.opcode.mnemonic = "BRn";
    line.flags           = 0x4;     // negative flag
    line.opcode.opcode   = 0x0;
    line.symbol          = "Done";
    line.label           = "TestEnd";
    line.is_label        = true;
    line.imm             = 0x3008;  // addr of 'Done'
    info.add(line);
    // Line 11 (ADD R3,R3,R4)
    initLineInfo(line);
    line.line_num        = 11;
    line.addr            = 0x3004;
    line.opcode.mnemonic = "ADD";
    line.opcode.opcode   = 0x01;
    line.arg1            = 3;
    line.arg2            = 3;
    line.arg3            = 4;
    info.add(line);
    // Line 12 (ADD, R1, R1, #1)
    initLineInfo(line);
    line.line_num        = 12;
    line.addr            = 0x3005;
    line.opcode.mnemonic = "ADD";
    line.opcode.opcode   = 0x01;
    line.arg1            = 1;
    line.arg2            = 1;
    line.imm             = 1;
    line.is_imm          = true;
    info.add(line);
    // Line 13 (LDR, R4, R1, #0)
    initLineInfo(line);
    line.line_num        = 13;
    line.addr            = 0x3006;
    line.opcode.mnemonic = "LDR";
    line.opcode.opcode   = 0x06;
    line.arg1            = 4;
    line.arg2            = 1;
    line.imm             = 0;
    line.is_imm          = true;
    info.add(line);
    // Line 14 (BRnzp TestEnd)
    initLineInfo(line);
    line.line_num        = 14;
    line.addr            = 0x3007;
    line.opcode.mnemonic = "BRnzp";
    line.flags           = 0x7;     // all flags
    line.opcode.opcode   = 0x0;
    line.symbol          = "TestEnd";
    line.imm             = 0x3003;
    info.add(line);
    // Line 16 (Done: Halt)
    initLineInfo(line);
    line.line_num        = 16;
    line.addr            = 0x3008;
    line.opcode.mnemonic = "TRAP";
    line.opcode.opcode   = 0xF;
    line.imm             = 0x25;
    line.label           = "Done";
    line.is_label        = true;
    info.add(line);

    return info;
}

// Expected assembly for sentinel program
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

TEST_F(TestDisassembler, test_dis_sentinel)
{
    int status;
    std::string src_filename = "data/sentinel.asm";
    std::string out_filename = "data/sentinel.dis";
    // Prepare some assembly output
    SourceInfo lex_output;
    Lexer lexer(this->op_table, src_filename);
    lexer.setVerbose(false);
    lex_output = lexer.lex();
    Assembler as(lex_output);
    as.setVerbose(false);
    std::cout << "\t Assembling program in file " << src_filename << std::endl;
    as.assemble();

    // Write binary to disk
    Program prog = as.getProgram();
    prog.setVerbose(true);
    prog.build();
    status = prog.save(out_filename);
    ASSERT_EQ(0, status);

    // disassemble the output
    Disassembler dis;
    dis.setVerbose(true);
    status = dis.read(out_filename);
    ASSERT_EQ(0, status);
    std::cout << "Disassembling file [" << out_filename << "]" << std::endl;
    dis.disassemble();

    // Show the expected disassembly
    SourceInfo expected_info = get_sentinel_test_source_info();
    std::cout << "\t Expected disassembly for file " << src_filename << std::endl;
    for(unsigned int idx = 0; idx < expected_info.getNumLines(); ++idx)
        expected_info.printLine(idx);

    // Get this disassembly output and compare
    std::cout << "\t Output disassembly for file " << src_filename << std::endl;
    SourceInfo dis_source = dis.getSourceInfo();
    for(unsigned int idx = 0; idx < dis_source.getNumLines(); ++idx)
        dis_source.printLine(idx);

    // Compare 
    std::cout << "Checking disassembly output... " << std::endl;
    // TODO : just check opcodes for now 
    for(unsigned int idx = 0; idx < expected_info.getNumLines(); ++idx)
    {
        LineInfo dis_line = dis_source.get(idx);
        LineInfo exp_line = expected_info.get(idx);
        std::cout << "Checking line " << idx << "(source line " << std::dec << dis_line.line_num << ") ...";
        std::cout << "<" << dis_line.opcode.mnemonic << "> (MNEMONIC ONLY)";
        //printLineDiff(dis_line, exp_line);
        // For now we skip the .ORIG directive in the expected source 
        if(exp_line.opcode.mnemonic == ".ORIG")
            continue;
        ASSERT_EQ(exp_line.opcode.mnemonic, dis_line.opcode.mnemonic);
        std::cout << std::endl;
    }

    //for(unsigned int idx = 0; idx < expected_info.getNumLines(); ++idx)
    //{
    //    LineInfo dis_line = dis_source.get(idx);
    //    LineInfo exp_line = expected_info.get(idx);
    //    std::cout << "Checking line " << idx << "(source line " << std::dec << dis_line.line_num << ") ...";
    //    std::cout << "<" << dis_line.opcode.mnemonic << ">";
    //    printLineDiff(dis_line, exp_line);
    //    ASSERT_EQ(true, compLineInfo(dis_line, exp_line));
    //    std::cout << " done" << std::endl;
    //}
}

// Test the line_to_asm() method
TEST_F(TestDisassembler, test_sentinel_dis_string)
{
    int status;
    std::string src_filename = "data/sentinel.asm";
    std::string out_filename = "data/sentinel.dis";
    // Prepare some assembly output
    SourceInfo lex_output;
    Lexer lexer(this->op_table, src_filename);
    lexer.setVerbose(false);
    lex_output = lexer.lex();
    Assembler as(lex_output);
    as.setVerbose(false);
    std::cout << "\t Assembling program in file " << src_filename << std::endl;
    as.assemble();
    // Write binary to disk
    Program prog = as.getProgram();
    prog.setVerbose(true);
    prog.build();
    status = prog.save(out_filename);
    ASSERT_EQ(0, status);

    // disassemble the output
    Disassembler dis;
    dis.setVerbose(true);
    status = dis.read(out_filename);
    ASSERT_EQ(0, status);
    std::cout << "Disassembling file [" << out_filename << "]" << std::endl;
    dis.disassemble();

    // For each line in the output, print the disassembly
    SourceInfo dis_output = dis.getSourceInfo();
    std::cout << "Dumping disassembly string for file " << src_filename << std::endl;
    for(unsigned int idx = 0; idx < dis_output.getNumLines(); ++idx)
    {
        LineInfo cur_line = dis_output.get(idx);
        std::cout << dis.line_to_asm(cur_line);
    }
}


std::string get_expected_add_dis_string(void)
{
    std::ostringstream oss;

    oss << "LD R1, #3007" << std::endl;
    oss << "LD R2, #3008" << std::endl;
    oss << "ADD R3, R1, R2" << std::endl;

    return oss.str();
}

TEST_F(TestDisassembler, test_add_dis_string)
{
    int status;
    std::string src_filename = "data/add_test.asm";
    std::string out_filename = "data/add_test.dis";
    // Prepare some assembly output
    SourceInfo lex_output;
    Lexer lexer(this->op_table, src_filename);
    lexer.setVerbose(false);
    lex_output = lexer.lex();
    Assembler as(lex_output);
    as.setVerbose(false);
    std::cout << "\t Assembling program in file " << src_filename << std::endl;
    as.assemble();
    // Write binary to disk
    Program prog = as.getProgram();
    prog.setVerbose(true);
    prog.build();
    status = prog.save(out_filename);
    ASSERT_EQ(0, status);

    // disassemble the output
    Disassembler dis;
    dis.setVerbose(true);
    status = dis.read(out_filename);
    ASSERT_EQ(0, status);
    std::cout << "Disassembling file [" << out_filename << "]" << std::endl;
    dis.disassemble();

    Program dis_program = dis.getProgram();
    std::vector <Instr> dis_instructions = dis_program.getInstr();

    // Dump raw program output
    std::cout << " N     ADDR   DATA " << std::endl;
    for(unsigned int idx = 0; idx < dis_instructions.size(); idx++)
    {
        std::cout << "[" << std::dec << std::setw(4) << std::setfill('0') << idx << "]";
        std::cout << " $" << std::hex << std::setw(4) << std::setfill('0') << dis_instructions[idx].adr;
        std::cout << "  " << std::hex << std::setw(4) << std::setfill('0') << dis_instructions[idx].ins;
        std::cout << std::endl;
    }

    // For each line in the output, print the disassembly
    SourceInfo dis_output = dis.getSourceInfo();
    std::cout << "Dumping disassembly string for file " << src_filename << std::endl;
    for(unsigned int idx = 0; idx < dis_output.getNumLines(); ++idx)
    {
        LineInfo cur_line = dis_output.get(idx);
        std::cout << dis.line_to_asm(cur_line);
    }
}

// TODO : need better tools before this is re-instated
//TEST_F(TestDisassembler, test_dis_file)
//{
//    int status;
//    // Prepare some assembly output
//    Lexer lexer(this->op_table, this->asm_src_filename);
//    lexer.setVerbose(false);
//    this->source_info = lexer.lex();
//    Assembler as(this->source_info);
//    as.setVerbose(false);
//    as.assemble();
//
//    // Write binary to disk
//    Program prog = as.getProgram();
//    prog.setVerbose(true);
//    prog.build();
//    status = prog.save(this->bin_output_filename);
//    ASSERT_EQ(0, status);
//
//    // disassemble the output
//    Disassembler dis;
//    dis.setVerbose(true);
//    status = dis.read(this->bin_output_filename);
//    ASSERT_EQ(0, status);
//    std::cout << "Disassembling" << std::endl;
//    dis.disassemble();
//
//    // Get the resulting SourceInfo and display
//    SourceInfo dsource = dis.getSourceInfo();
//    for(unsigned int idx = 0; idx < dsource.getNumLines(); ++idx)
//        dsource.printLine(idx);
//}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
        //s
}
