/* LC3ASM
 * Entryu point for the LC3 assembler
 *
 * Stefan Wong 2018
 */

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <getopt.h>
#include "lc3.hpp"
#include "opcode.hpp"
#include "source.hpp"
#include "lexer.hpp"
#include "assembler.hpp"

// TODO : help output


// TODO : the 'true' op table should come from an LC3 machine object 
OpcodeTable test_build_op_table(void)
{
    OpcodeTable op_table;

    // iterate over this in the C++ way
    for(const Opcode &op : lc3_op_list)
        op_table.add(op);

    return op_table;
}

typedef struct 
{
    std::string in_filename;
    std::string out_filename;
    int errors;
    bool verbose;
} AsmArgs;

void init_cmd_args(AsmArgs& args)
{
    args.in_filename = "\0";
    args.out_filename = "out.asm";
    args.errors = 0;
    args.verbose = false;
}

AsmArgs get_cmd_args(int argc, char *argv[])
{
    AsmArgs args;
    const char* const short_opts = "vhi:o:";
    const option long_opts[] = {};
    int argn = 0;

    init_cmd_args(args);

    // Parse arguments 
    while(1)
    {
        const auto opt = getopt_long(argc, argv, short_opts, long_opts, nullptr);
        if(opt == -1)
            break;
        switch(opt)
        {
            case 'v':
                args.verbose = true;
                std::cout << "Set verbose mode" << std::endl;
                break;

            case 'h':
                std::cout << "Would print help here and exit" << std::endl;
                exit(0);

            case 'i':
                args.in_filename = std::string(optarg);
                break;

            case 'o':
                args.out_filename = std::string(optarg);
                break;

            default:
                std::cout << "Unknown option " << std::string(optarg)
                    << " (arg " << argn << ") - would print help here " << std::endl;
                break;
        }
        argn++;
    }

    return args;
}


int main(int argc, char *argv[])
{
    AsmArgs args;
    
    args = get_cmd_args(argc, argv);
    if(args.errors > 0)
    {
        std::cout << "Error parsing args" << std::endl;
        return -1;
    }

    // Check args
    if(args.in_filename == "\0")
    {
        std::cout << "Error: no input filename specified" << std::endl;
        return -1;
    }

    // Get a lexer
    OpcodeTable op_table = test_build_op_table();       // TODO : this should eventually come from machine
    Lexer lexer(op_table, args.in_filename);
    lexer.setVerbose(args.verbose);
    SourceInfo src;

    // TODO : handle errors between parts 
    
    if(args.verbose)
        std::cout << "Lexing source file " << args.in_filename << std::endl;

    src = lexer.lex();

    Assembler assem(src);
    assem.setVerbose(args.verbose);
    if(args.verbose)
        std::cout << "Assembling to output file " << args.out_filename << std::endl;
    assem.assemble();
    assem.write(args.out_filename);

    return 0;
}
