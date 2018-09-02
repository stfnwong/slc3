/* 
 * LC3DIS
 * LC3 Disassembler
 *
 * Stefan Wong 2018
 */

#include <iostream>
#include <fstream>
#include <string>
#include <getopt.h>      // for getopt

#include "lexer.hpp"
#include "disassembler.hpp"
#include "source.hpp"


typedef struct
{
    std::string in_file;
    std::string out_file;
    bool verbose;
} DisArgs;


void init_cmd_args(DisArgs& args)
{
    args.in_file = "\0";
    args.out_file = "out.s";
    args.verbose = false;
}

DisArgs get_cmd_args(int argc, char *argv[])
{
    DisArgs args; 
    const char* const short_opts = "vhi:o:";
    const option long_opts[] = {};
    int argn = 0;

    init_cmd_args(args);

    while(1)
    {
        const auto opt = getopt_long(argc, argv, short_opts, long_opts, nullptr);
        if(opt == -1)
            break;
        switch(opt)
        {
            case 'v':
                args.verbose = true;
                break;

            case 'h':
                std::cout << "TODO : Display help here " << std::endl;
                exit(0);

            case 'i':
                args.in_file = std::string(optarg);
                break;

            case 'o':
                args.out_file = std::string(optarg);

        }
        argn++;
    }

    return args;
}

int main(int argc, char *argv[])
{
    DisArgs args;
    Disassembler dis;
    int status;

    args = get_cmd_args(argc, argv);

    if(args.in_file == "\0")
    {
        std::cout << "Error: no input filename " << std::endl;
        return -1;
    }

    dis.setVerbose(args.verbose);
    status = dis.read(args.in_file);
    if(status < 0)
    {
        std::cout << "Error reading file " << args.in_file << std::endl;
        return -1;
    }

    dis.disassemble();

    // Open text file here and write lines
    std::ofstream out(args.out_file);

    SourceInfo dis_output = dis.getSourceInfo();
    for(unsigned int idx = 0; idx < dis_output.getNumLines(); ++idx)
    {
        LineInfo cur_line = dis_output.get(idx);
        out << dis.line_to_asm(cur_line);
        if(args.verbose)
            std::cout << dis.line_to_asm(cur_line);
    }

    out.close();

    return 0;
}
