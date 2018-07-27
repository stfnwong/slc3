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
// TODO : I thikn I need to make an abstract Lexer class and then
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
    this->cur_pos            = 0;
    this->token_buf_size = 64;
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

void Lexer::readAlphaNum(void)
{
    unsigned int idx = 0;
    while(!this->isSpace() && idx < (this->token_buf_size-1))
    {
        this->token_buf[idx] = this->cur_char;
        this->advance();
        idx++;
    }
    this->token_buf[idx+1] = '\0';
}

bool Lexer::isAlphaNum(void)
{
    return (isalnum(toupper(this->cur_char)));
}

bool Lexer::isSpace(void)
{
    return (this->cur_char == ' '  || 
            this->cur_char == '\t' ||
            this->cur_char == '\n') ? true : false;
}

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


Opcode Lexer::parseOpcode(void)
{
    Opcode o;

    this->op_table.get(std::string(this->token_buf), o);
    if(o.mnemonic == "M_INVALID")
        return o;

    // TODO : How to generically parse Opcodes?  Maybe I need virtual 
    // methods here 
    switch(o.opcode)
    {
        case LC3_ADD:
            std::cout << "Got an ADD" << std::endl;
            break;
        case LC3_AND:
            std::cout << "Got an AND" << std::endl;
            break;
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

void Lexer::lex(void)
{
    this->cur_line = 1;
    this->cur_pos      = 0;
    std::cout << "Starting lexer..." << std::endl;
    LineInfo line_info;

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
        //if(this->isSpace())
        //    this->skipWhitespace();

        // Skip comments
        if(this->cur_char ==  ';')
        {
            if(this->verbose)
                std::cout << "Found a comment on line " << this->cur_line << std::endl;
            this->skipComment();
            //continue;
        }

        // Check for directives
        if(this->cur_char == '.')
        {
            if(this->verbose)
                std::cout << "Found a directive on line " << this->cur_line;
            this->readAlphaNum();
            initLineInfo(line_info);
            line_info.is_directive = true;
            line_info.line_num = this->cur_line;
            // TODO: replace with C++ string compare idiom
            // TODO: replace print with real action
            if(strncmp(this->token_buf, LEX_ORIG, 5))
            {
                std::cout << "[Lexer] (line " << 
                    this->cur_line << ") found directive " << 
                    std::string(this->token_buf) << std::endl;
                line_info.symbol = LEX_ORIG;
            }
            else if(strncmp(this->token_buf, LEX_END, 4))
            {
                std::cout << "[Lexer] (line " << 
                    this->cur_line << ") found directive " << 
                    std::string(this->token_buf) << std::endl;
                line_info.symbol = LEX_END;
            }
            else if(strncmp(this->token_buf, LEX_BLKW, 5))
            {
                std::cout << "[Lexer] (line " << 
                    this->cur_line << ") found directive " << 
                    std::string(this->token_buf) << std::endl;
                line_info.symbol = LEX_BLKW;
            }
            else if(strncmp(this->token_buf, LEX_FILL, 5))
            {
                std::cout << "[Lexer] (line " << 
                    this->cur_line << ") found directive " << 
                    std::string(this->token_buf) << std::endl;
                line_info.symbol = LEX_FILL;
            }
            else if(strncmp(this->token_buf, LEX_STRINGZ, 8))
            {
                std::cout << "[Lexer] (line " << 
                    this->cur_line << ") found directive " << 
                    std::string(this->token_buf) << std::endl;
                line_info.symbol = LEX_STRINGZ;
            }
            else
            {
                std::cout << "[Lexer] (line " << this->cur_line 
                    << ")  found invalid directive " 
                    << std::string(this->token_buf) << std::endl;
                std::cout << "Lexer would exit here" << std::endl;
                line_info.symbol = LEX_INVALID;
            }
            //TODO : try to get the value/symbol out
            std::cout << "[Lexer] would try to get directive value here" << std::endl;
            //this->skipWhitespace();     // TODO : parse symbol
            this->skipLine();
        }

        // Check for symbols
        if(this->isAlphaNum())
        {
            this->readAlphaNum();
            if(this->verbose)
            {
                std::cout << "Found a symbol [" << 
                    std::string(this->token_buf) << 
                    "] on line " << this->cur_line << std::endl;
            }
            initLineInfo(line_info);
            if(!this->isMnemonic())
            {
                line_info.is_label = true;
                line_info.line_num = this->cur_line;
                line_info.label    = std::string(this->token_buf);
            }

            // Now parse the rest of the line
            // What is the mnemonic?
            this->skipWhitespace();
            this->readAlphaNum();
            if(this->isMnemonic())
            {
                line_info.opcode = this->parseOpcode();
            }

            std::cout << "[Lexer] would parse opcode here " << std::endl;
            this->skipLine();
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

// TODO: debug, remove 
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
