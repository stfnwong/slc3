/* LEXER
 * Lexer object for scanning assembly files and extracting
 * tokens.
 * 
 * Stefan Wong 2018
 */

#include <iostream>
#include <fstream>
#include <cstdlib>  // for std::atoi()
#include <cctype>   // for std::toupper, std::isalnum
#include <cstring>  // for strncmp()
#include "lexer.hpp"
// TODO : I think I need to make an abstract Lexer class and then
// derive an LC3 class to make this 'generic'
#include "lc3.hpp"

// LineInfo
void initLineInfo(LineInfo& l)
{
    l.line_num = 0;
    l.addr     = 0;
    l.symbol   = "\0";
    l.label    = "\0";
    l.opcode   = {0x0, "DEFAULT"},
    l.dest     = 0;
    l.src1     = 0;
    l.src2     = 0;
    l.imm      = 0;
    l.is_imm   = false;
    l.is_label = false;
    l.error    = false;
    l.is_directive = false;
}

/*
 * SOURCEINFO 
 */
SourceInfo::SourceInfo() {} 

SourceInfo::~SourceInfo() {} 

void SourceInfo::add(const LineInfo& l)
{
    this->line_info.push_back(l);
}

LineInfo SourceInfo::get(const unsigned int idx) const
{
    if(idx < this->line_info.size())
        return this->line_info[idx];
    else
    {
        LineInfo l;
        l.line_num = 0;
        
        return l;
    }
}

unsigned int SourceInfo::getLineNum(const unsigned int idx) const
{
    return this->line_info[idx].line_num;
}

unsigned int SourceInfo::getNumLines(void) const
{
    return this->line_info.size();
}

unsigned int SourceInfo::getNumError(void) const
{
    unsigned int num_err = 0;
    
    for(unsigned int idx = 0; idx < this->line_info.size(); idx++)
    {
        if(this->line_info[idx].error)
            num_err++;
    }

    return num_err;
}

/*
 * LEXER 
 */
Lexer::Lexer(const OpcodeTable& ot)
{
    this->op_table = ot;
    this->initVars();
    this->allocMem();
}

Lexer::Lexer(const OpcodeTable& ot, const std::string& filename)
{
    this->loadFile(filename);
    this->op_table = ot;
    this->initVars();
    this->allocMem();
}

Lexer::~Lexer()
{
    delete[] this->token_buf;
}

void Lexer::allocMem(void)
{
    this->token_buf = new char[this->token_buf_size];
    this->token_buf[0] = '\0';
}

void Lexer::initVars(void)
{
    this->verbose        = true;  // TODO: should be false in final ver
    this->cur_pos        = 0;
    this->token_buf_size = LEX_TOKEN_MAX_LEN;
    this->cur_line       = 0;
}

/*
 * advance()
 *
 * Move the character pointer through the source, 
 * returning a null character when the source file \
 * is exhausted.
 */
void Lexer::advance(void)
{
    this->cur_pos++;
    this->cur_char = this->src[this->cur_pos];
    if(this->cur_pos >= this->src.size())
        this->cur_char = '\0';
    if(this->cur_char == '\n')
        this->cur_line++;
}

bool Lexer::exhausted(void) const
{
    return (this->cur_char == '\0' ||
            this->cur_pos >= this->src.size()) ? true : false;
}

// TODO : something is up with isSpace()
void Lexer::skipWhitespace(void) 
{
    bool space = true;
    while(space)
    {
        this->advance();
        space = (this->cur_char == ' ' ||
                 this->cur_char == '\n' || 
                 this->cur_char == '\t') ? true : false;
    }

}

void Lexer::skipComment(void)
{
    while(this->cur_char != '\n')
        this->advance();
}

void Lexer::readSymbol(void)
{
    unsigned int idx = 0;
    while(!this->isSpace() && idx < (this->token_buf_size-1))
    {
        this->token_buf[idx] = this->cur_char;
        this->advance();
        idx++;
    }
    this->token_buf[idx] = '\0';

    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] : token_buf contains <" << 
            std::string(this->token_buf) << "> " << std::endl;
    }
}

bool Lexer::isSymbol(void) const
{
    return (isalnum(toupper(this->cur_char)));
}

bool Lexer::isDirective(void) const 
{
    // TODO : This feels like a hack... there might be an
    // extra 'advance()' somewhere
    return (this->cur_char == '.' || this->token_buf[0] == '.');
}

bool Lexer::isSpace(void)
{
    return (this->cur_char == ' '  || 
            this->cur_char == '\t' ||
            this->cur_char == '\n') ? true : false;
}

// TODO: need to deal with case (removing case) here 
bool Lexer::isMnemonic(void)
{
    if(this->token_buf[0] == '\0')
        return false;
    Opcode op;
    this->op_table.get(std::string(this->token_buf), op);

    return (op.mnemonic != "M_INVALID") ? true : false;
}

void Lexer::skipLine(void)
{
    while(this->cur_char != '\n')
        this->advance();
    // skip ahead one
    this->advance();
}

bool Lexer::getNextArg(void)
{
    unsigned int idx = 0;
    while(this->cur_char != ',')
    {
        if(this->cur_char == '\n')
            break;
        this->token_buf[idx] = this->cur_char;
        this->advance();
        idx++;
    }

    this->advance();
    this->token_buf[idx] = '\0';
    if(this->token_buf[0] == 'r' ||
       this->token_buf[0] == 'R' || 
       this->token_buf[0] == '#')
    {
        std::cout << "[" << __FUNCTION__ << "] valid arg <" <<
            std::string(this->token_buf) << ">" << std::endl;
        return true;
    }
    else
    {
        std::cout << "[" << __FUNCTION__ << "] invalid arg <" <<
            std::string(this->token_buf) << ">" << std::endl;
        return false;
    }
}

void Lexer::parseOpcodeArgs(void)
{

}

LineInfo Lexer::parseDirective(void)
{
    LineInfo info;
    
    initLineInfo(info);
    info.is_directive = true;
    info.line_num     = this->cur_line;
    info.opcode.mnemonic     = "DIRECTIVE";
    this->readSymbol();

    // Note: these symbols are also LC3 assembly specific, 
    // in the generic version they will need to be moved out 
    // to some other class / struct
    if(std::string(this->token_buf) == LEX_ORIG)
        info.symbol = LEX_ORIG;
    else if(std::string(this->token_buf) == LEX_END)
        info.symbol = LEX_END;
    else if(std::string(this->token_buf) == LEX_BLKW)
        info.symbol = LEX_BLKW;
    else if(std::string(this->token_buf) == LEX_FILL)
        info.symbol = LEX_FILL;
    else if(std::string(this->token_buf) == LEX_STRINGZ)
        info.symbol = LEX_STRINGZ;
    else
        info.symbol = LEX_INVALID;

    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] (line " << 
            this->cur_line << ") extracted directive symbol " <<
            std::string(this->token_buf) << " and set info.symbol as <"
            << info.symbol << ">" << std::endl;
    }
    
    if(info.symbol == LEX_INVALID)
    {
        info.error = true;
        return info;
    }

    if(info.symbol == LEX_END)
        return info;

    // Try to read the arg (if any)
    this->skipWhitespace();
    if(this->isSymbol())
    {
        this->readSymbol();
        if(this->verbose)
        {
            std::cout << "[" << __FUNCTION__ << 
                "] found directive symbol <" 
                << std::string(this->token_buf) << ">" << std::endl;
        }
        if(this->token_buf[0] == '#')
            info.imm = std::stoi(this->token_buf + 1);
        else if(this->token_buf[0] == 'x' || this->token_buf[0] == 'X')
            info.imm = std::stoi("0" + std::string(this->token_buf));
        else
            info.imm = std::stoi(std::string(this->token_buf));

        std::cout << "[" << __FUNCTION__ << "] set imm to <" <<
            info.imm << ">" << std::endl;
    }

    this->skipLine();

    return info;
}

/*
 * parseLine()
 * Parse a single line of the assembly source
 */
LineInfo Lexer::parseLine(void)
{
    LineInfo info;
    Opcode o;

    initLineInfo(info);   
    info.line_num = this->cur_line;
    std::cout << "[" << __FUNCTION__ << "] token_buf contains <" << 
        std::string(this->token_buf) << ">" << std::endl;

    // Could be a directive 
    if(this->isDirective())
    {
        std::cout << "[" << __FUNCTION__ << "] (line " << 
            this->cur_line << ") found directive <" << 
            std::string(this->token_buf) << ">" << std::endl;
        info = this->parseDirective();
        return info;
    }


    // Not a directive, load symbol into token_buf and
    // check against known opcodes
    this->readSymbol();
    if(!this->isMnemonic())
    {
        // If not mnemonic then must be label
        info.is_label = true;
        if(std::string(this->token_buf) == "HALT")
        {
            if(this->verbose)
            {
                std::cout << "[" << __FUNCTION__ << "] got HALT (line " 
                    << this->cur_line << ")" << std::endl;
            }
            return info;
        }

        if(this->verbose)
        {
            std::cout << "[" << __FUNCTION__ << "] (line " << 
                this->cur_line << ") found label symbol <" 
                << std::string(this->token_buf) << ">" << std::endl;
        }

        info.label = std::string(this->token_buf);
        //this->advance();        // skip past the ':'
        this->skipWhitespace();

        // We may have a labelled directive
        if(this->isDirective())
        {
            std::cout << "[" << __FUNCTION__ << "] (line " <<
                this->cur_line << ") found labelled directive <" << 
                std::string(this->token_buf) << ">" << std::endl;

            LineInfo dir_info;
            initLineInfo(dir_info);
            dir_info = this->parseDirective();
            info.is_directive = true;
            info.symbol       = dir_info.symbol;
            info.imm          = dir_info.imm;
            info.line_num     = dir_info.line_num;
            info.opcode       = dir_info.opcode;

            return info;
        }
        this->readSymbol();
    }

    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] (line " << this->cur_line << 
            ") found opcode symbol " << std::string(this->token_buf) 
            << std::endl;
    }

    this->op_table.get(std::string(this->token_buf), o);
    info.opcode = o;
    if(o.mnemonic == "M_INVALID")
    {
        if(this->verbose)
        {
            std::cout << "[" << __FUNCTION__ << "] opcode " <<
                std::string(this->token_buf) << " not a valid opcode" << std::endl;
        }
        info.error = true;
        return info;
    }
    this->skipWhitespace();


    // TODO : How to generically parse Opcodes?  Maybe I need virtual 
    // methods here 
    int err_argnum = 0;
    bool arg_err = false;
    std::string arg;

    switch(o.opcode)
    {
        case LC3_BR:
            std::cout << "[" << __FUNCTION__ << "] got BR" << std::endl;
            std::cout << "BR Not yet implemented" << std::endl;
            this->skipLine();

            break;

        case LC3_ADD:
            // 3 args, comma seperated (DST, SR1, SR2)
            std::cout << "[" << __FUNCTION__ << "] got ADD" << std::endl;

            // Get dest
            if(!this->getNextArg())
            {
                err_argnum = 1;
                arg_err = true;
                goto LC3_ADD_ARG_ERR;
            }
            arg = std::string(this->token_buf);
            info.dest = std::stoi(arg.substr(1, arg.length()));

            // Get source 1
            if(!this->getNextArg())
            {
                err_argnum = 2;
                arg_err = true;
                goto LC3_ADD_ARG_ERR;
            }
            arg = std::string(this->token_buf);
            info.src1 = std::stoi(arg.substr(1, arg.length()));

            // Get source 2
            if(!this->getNextArg())
            {
                err_argnum = 3;
                arg_err = true;
                goto LC3_ADD_ARG_ERR;
            }
            arg = std::string(this->token_buf);
            if(arg[0] == '#')
            {
                info.imm = std::stoi(arg.substr(1, arg.length()));
                info.is_imm = true;
            }
            else
                info.src2 = std::stoi(arg.substr(1, arg.length()));

            break;

        case LC3_AND:
            std::cout << "[" << __FUNCTION__ << "] got AND" << std::endl;
            this->getNextArg();
            std::cout << "[" << __FUNCTION__ << "] got AND arg1 : " << std::string(this->token_buf) << std::endl;
            //info.dest = std::stoi(this->token_buf + 1);
            this->getNextArg();
            std::cout << "[" << __FUNCTION__ << "] got AND arg2 : " << std::string(this->token_buf) << std::endl;
            //info.src1 = std::stoi(this->token_buf + 1);
            // Last arg could be imm
            this->getNextArg();
            if(this->token_buf[0] == '#')
            {
                std::cout << "[" << __FUNCTION__ << "] got AND arg3 (# immediate) : " << std::string(this->token_buf) << std::endl;
                info.imm = std::stoi(this->token_buf + 1);
            }
            else
            {
                std::cout << "[" << __FUNCTION__ << "] got AND arg3 : " << std::string(this->token_buf) << std::endl;
                info.imm = 0;
            }
            this->skipLine();
            break;

        case LC3_LD:
            std::cout << "[" << __FUNCTION__ << "] got LD" << std::endl;
            // TODO  : implement
            std::cout << "LD not yet implemented, skipping" << std::endl;
            this->skipLine();
            break;

        case LC3_LDR:
            std::cout << "[" << __FUNCTION__ << "] got LDR" << std::endl;
            // TODO  : implement
            std::cout << "LDR not yet implemented, skipping" << std::endl;
            this->skipLine();
            break;

        case LC3_LEA:
            std::cout << "[" << __FUNCTION__ << "] got LEA" << std::endl;
            // TODO  : implement
            std::cout << "LEA not yet implemented, skipping" << std::endl;
            this->skipLine();
            break;

        case LC3_STR:
            std::cout << "[" << __FUNCTION__ << "] got STR" << std::endl;
            // TODO  : implement
            std::cout << "STR not yet implemented, skipping" << std::endl;
            this->skipLine();
            break;

            // Handle argument parsing
LC3_ADD_ARG_ERR:
            if(arg_err)
            {
                std::cout << "[" << __FUNCTION__ << "] (line " <<
                    this->cur_line << ") error parsing arg " <<
                    err_argnum << " of ADD opcode" << std::endl;
                info.error = true;
                this->skipLine();
            }
            break;

        default:
            std::cout << "[" << __FUNCTION__ << "] invalid opoode <" 
                << std::hex << o.opcode << std::endl;
            break;

    }

    return info;
}



// Do lexing pass
void Lexer::lex(void)
{
    this->cur_line = 1;
    this->cur_pos  = 0;
    LineInfo line;

    while(!this->exhausted())
    {
        // Skip whitespace 
        if(this->isSpace())
        {
            this->advance();
            continue;
        }

        // Skip comments
        if(this->cur_char ==  ';')
        {
            if(this->verbose)
                std::cout << "Found a comment on line " << this->cur_line << std::endl;
            //this->skipComment();
            this->skipLine();
            continue;
        }

        // Check for symbols or directives
        if(this->isSymbol() ||  this->isDirective())
        {
            line = this->parseLine();
            this->source_info.add(line);
            continue;
        }
    }
}

// ==== FILE LOADING
void Lexer::loadFile(const std::string& filename)
{
    std::ifstream infile(filename);
    std::string line;

    // save the filename
    this->filename = filename;
    while(std::getline(infile, line))
    {
        this->src += line;
        this->src.push_back('\n');
    }

    infile.close();
    this->src.push_back('\0');
    this->cur_char = this->src[0];
}

// ==== Getters 
unsigned int Lexer::getSrcLength(void) const
{
    return this->src.size();
}

std::string Lexer::getFilename(void) const
{
    return this->filename;
}

std::string Lexer::dumpSrc(void) const
{
    return this->src;
}


// Verbose 
void Lexer::setVerbose(const bool b)
{
    this->verbose = b;
}

bool Lexer::getVerbose(void) const
{
    return this->verbose;
}

bool Lexer::isASCII(void) const
{
    for(unsigned int idx = 0; idx < this->src.size(); idx++)
    {
        if(src[idx] > 127)
            return false;
    }
    
    return true;
}

char Lexer::dumpchar(const unsigned int idx) const
{
    if(idx < this->src.size())
        return this->src[idx];
    
    return '\0';
}

OpcodeTable Lexer::dumpOpTable(void) const
{
    return this->op_table;
}

unsigned int Lexer::opTableSize(void) const
{
    return this->op_table.getNumOps();
}

SourceInfo Lexer::dumpSrcInfo(void) const
{
    return this->source_info;
}
