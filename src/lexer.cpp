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
    //return (this->cur_char == '\0') ? true : false;
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
    //while(this->isSpace())
    //    this->advance();

    // advance one more to se the cur_pos just ahead of whitespace 
    this->advance();
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

bool Lexer::isSymbol(void)
{
    return (isalnum(toupper(this->cur_char)));
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

    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] isMnemonic found mnemonic " << 
            op.mnemonic << std::endl;
        std::cout << "From token_buf " << std::string(this->token_buf) 
            << std::endl;
    }

    return (op.mnemonic != "M_INVALID") ? true : false;
}

void Lexer::skipLine(void)
{
    while(this->cur_char != '\n')
        this->advance();
    // skip ahead one
    this->advance();
}

LineInfo Lexer::parseDirective(void)
{
    LineInfo info;
    
    initLineInfo(info);
    this->readSymbol();
    info.is_directive = true;
    info.line_num     = this->cur_line;

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
            this->cur_line << ") extracted symbol " <<
            std::string(this->token_buf) << std::endl;
    }

    if(info.symbol == LEX_END)
        return info;

    this->skipWhitespace();
    bool is_hex = false;
    size_t imm;
    if(this->isSymbol())
    {
        // TODO : re-write this more elegantly
        if(this->cur_char == 'x' || this->cur_char == 'X')
            is_hex = true;
        else if(this->cur_char == 'd' || this->cur_char == 'D')
            is_hex = false;
        this->advance();
        this->readSymbol();

        if(this->verbose)
        {
            std::cout << "[" << __FUNCTION__ << "] found directive symbol " 
                << std::string(this->token_buf) << std::endl;
            std::cout << "(would be) Attempting to parse symbol <" 
                << std::string(this->token_buf) << "> as a arg"
                << std::endl;
        }


        //if(is_hex)
        //    std::stoi(std::string(this->token_buf), &imm, 16);
        //else
        //    std::stoi(std::string(this->token_buf), &imm, 10);
        //info.imm = (uint16_t) imm;
    }

    this->skipLine();

    return info;
}

LineInfo Lexer::parseSymbol(void)
{
    LineInfo info;
    Opcode o;

    // TODO : move other line stuff here
    initLineInfo(info);   
    this->op_table.get(std::string(this->token_buf), o);
    info.opcode = o;
    if(o.mnemonic == "M_INVALID")
        return info;

    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] line " << this->cur_line << 
            " found mnemonic symbol " << std::string(this->token_buf) 
            << std::endl;
    }

    // TODO : How to generically parse Opcodes?  Maybe I need virtual 
    // methods here 
    switch(o.opcode)
    {
        case LC3_ADD:
            // 3 args, comma seperated (DST, SR1, SR2)
            // SR2 could be imm, so look for '#' character
            this->parseNextArg();
            std::cout << "[" << __FUNCTION__ << "] got ADD arg1 : " << std::string(this->token_buf) << std::endl;
            this->parseNextArg();
            std::cout << "[" << __FUNCTION__ << "] got ADD arg2 : " << std::string(this->token_buf) << std::endl;
            this->parseNextArg();
            if(this->token_buf[0] == '#')
                std::cout << "[" << __FUNCTION__ << "] got ADD arg3 (# immediate) : " << std::string(this->token_buf) << std::endl;
            else
                std::cout << "[" << __FUNCTION__ << "] got ADD arg3 : " << std::string(this->token_buf) << std::endl;
            break;

        case LC3_AND:
            std::cout << "Got an AND" << std::endl;
            break;
    }

    return info;
}

LineInfo Lexer::parseLabelSymbol(void)
{
    LineInfo info;
    line_info.is_label = true;
    line_info.line_num = this->cur_line;
    line_info.label    = std::string(this->token_buf);

    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] line " << this->cur_line << 
            " Found label " << std::string(this->token_buf) 
            << std::endl;
    }

    return info;
}

void Lexer::parseNextArg(void)
{
    unsigned int idx = 0;
    while(this->cur_char != ',')
    {
        this->token_buf[idx] = this->cur_char;
        this->advance();
        idx++;
    }
    // advance one character ahead
    this->advance();
    this->token_buf[idx] = '\0';   // changed from idx+1
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

void Lexer::lex(void)
{
    this->cur_line = 1;
    this->cur_pos  = 0;

    while(!this->exhausted())
    {
        // Skip whitespace 
        if(this->cur_char == ' '  || 
           this->cur_char == '\t' || 
           this->cur_char == '\n')
        {
            this->advance();
            continue;
        }

        // Skip comments
        if(this->cur_char ==  ';')
        {
            if(this->verbose)
                std::cout << "Found a comment on line " << this->cur_line << std::endl;
            this->skipComment();
        }

        // Check for directives
        if(this->cur_char == '.')
        {
            if(this->verbose)
                std::cout << "Found a directive on line " << this->cur_line;
            LineInfo line = this->parseDirective();
            this->source_info.add(line);
        }

        // Check for symbols
        if(this->isSymbol())
        {
            this->readSymbol();
            if(this->verbose)
            {
                std::cout << "[" << __FUNCTION__ << "] reading symbol <" << 
                    std::string(this->token_buf) << 
                    "> on line " << this->cur_line << std::endl;
            }
            // If not a known mnemonic, assume its a label
            if(this->isMnemonic())
                LineInfo line = this->parseSymbol();
            else
                LineInfo line = this->parseLabelSymbol();

        }
    }
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
