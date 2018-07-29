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
    this->verbose        = false;
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

bool Lexer::isNumber(void) const
{
    return (isdigit(toupper(this->cur_char)));
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
    return (this->token_buf[0] == 'r' ||
            this->token_buf[0] == 'R' || 
            this->token_buf[0] == '#' ||
            this->token_buf[0] == 'x' ||
            this->token_buf[0] == 'X') ? true : false;
}

void Lexer::parseOpcodeArgs(void)
{
    int err_argnum = 0;
    bool arg_err = false;
    std::string arg;
    // Get arg1
    if(!this->getNextArg())
    {
        err_argnum = 1;
        arg_err = true;
        goto ARG_ERR;
    }
    arg = std::string(this->token_buf);
    this->line_info.arg1 = std::stoi(arg.substr(1, arg.length()));

    // Get source 1
    if(!this->getNextArg())
    {
        err_argnum = 2;
        arg_err = true;
        goto ARG_ERR;
    }
    arg = std::string(this->token_buf);
    this->line_info.arg2 = std::stoi(arg.substr(1, arg.length()));

    // Get source 2
    if(!this->getNextArg())
    {
        err_argnum = 3;
        arg_err = true;
        goto ARG_ERR;
    }
    arg = std::string(this->token_buf);
    if(arg[0] == '#')
    {
        this->line_info.imm = std::stoi(arg.substr(1, arg.length()));
        this->line_info.is_imm = true;
    }
    else
        this->line_info.arg3 = std::stoi(arg.substr(1, arg.length()));

ARG_ERR:
    if(arg_err)
    {
        if(this->verbose)
        {
            std::cout << "[" << __FUNCTION__ << "] (line " <<
                this->cur_line << ") error parsing arg " <<
                err_argnum << " of ADD opcode" << std::endl;
        }
        this->line_info.error = true;
        this->skipLine();
    }
}

void Lexer::parseOpcode(const Opcode& o)
{
    // TODO : How to generically parse Opcodes?  Maybe I need virtual 
    // methods here 
    if(this->verbose)
        std::cout << "[" << __FUNCTION__ << "] decoding <" 
            << o.mnemonic << ">" << std::endl;
    
    std::string arg;
    switch(o.opcode)
    {
        case LC3_BR:

            // Check what (if any) flags are in opcode
            if(this->line_info.opcode.mnemonic.length() > 2)
            {
                unsigned int num_flags;
                num_flags = this->line_info.opcode.mnemonic.length() - 2;
                if(this->verbose)
                {
                    std::cout << "[" << __FUNCTION__ << "] BR has " << 
                        num_flags << " flag arguments" << std::endl;
                }

                for(unsigned int f = 0; f < num_flags; f++)
                {
                    if(this->line_info.opcode.mnemonic[f + 2] == 'p')
                        this->line_info.flags |= LC3_FLAG_P;
                    if(this->line_info.opcode.mnemonic[f + 2] == 'n')
                        this->line_info.flags |= LC3_FLAG_N;
                    if(this->line_info.opcode.mnemonic[f + 2] == 'z')
                        this->line_info.flags |= LC3_FLAG_Z;
                }
            }
            // There should be either an immediate or a label symbol;
            if(this->getNextArg())      // assume literal
            {
                arg = std::string(this->token_buf);
                if(this->token_buf[0] == 'x' || this->token_buf[0] == 'X')
                    this->line_info.imm = std::stoi("0" + std::string(this->token_buf));
                else
                    this->line_info.imm = std::stoi(std::string(this->token_buf));
            }
            else        // assume label symbol
                this->line_info.symbol = std::string(this->token_buf);

            this->skipLine();
            break;

        case LC3_ADD:
            // 3 args, comma seperated (DST, SR1, SR2)
            this->parseOpcodeArgs();
            break;

        case LC3_AND:
            // 3 args, comma seperated (DST, SR1, SR2)
            this->parseOpcodeArgs();
            this->skipLine();
            break;

        case LC3_LD:
            if(!this->getNextArg())
            {
                this->line_info.error = true;
                break;
            }
            arg = std::string(this->token_buf);
            this->line_info.arg1 = std::stoi(arg.substr(1, arg.length()));
            // Next could be a symbol
            if(this->isNumber())
            {
                this->readSymbol();
                this->line_info.imm = std::stoi(std::string(this->token_buf));
            }
            else
            {
                this->readSymbol();
                this->line_info.symbol = std::string(this->token_buf);
            }
            this->skipLine();
            break;

        case LC3_LDI:

            break;

        case LC3_LDR:
            // Get arg1 register 
            if(!this->getNextArg())
            {
                this->line_info.error = true;
                break;
            }
            arg = std::string(this->token_buf);
            this->line_info.arg1 = std::stoi(arg.substr(1, arg.length()));
            // Get base register 
            if(!this->getNextArg())
            {
                this->line_info.error = true;
                break;
            }
            arg = std::string(this->token_buf);
            this->line_info.arg2 = std::stoi(arg.substr(1, arg.length()));

            // Get offset6, which must be a literal
            if(!this->getNextArg())
            {
                this->line_info.error = true;
                break;
            }
            arg = std::string(this->token_buf);
            this->line_info.imm = std::stoi(arg.substr(1, arg.length()));
            this->line_info.is_imm = true;      // redundant?
            
            this->skipLine();
            break;

        case LC3_LEA:
            if(!this->getNextArg())
            {
                this->line_info.error = true;
                break;
            }
            arg = std::string(this->token_buf);
            this->line_info.arg1 = std::stoi(arg.substr(1, arg.length()));
            // Next could be a symbol
            if(this->isNumber())
            {
                this->readSymbol();
                this->line_info.imm = std::stoi(std::string(this->token_buf));
            }
            else
            {
                this->readSymbol();
                this->line_info.symbol = std::string(this->token_buf);
            }

            this->skipLine();
            break;

        case LC3_ST:
        case LC3_STR:
            // Get src register 
            if(!this->getNextArg())
            {
                this->line_info.error = true;
                break;
            }
            arg = std::string(this->token_buf);
            this->line_info.arg1 = std::stoi(arg.substr(1, arg.length()));
            // Get base register 
            if(!this->getNextArg())
            {
                this->line_info.error = true;
                break;
            }
            arg = std::string(this->token_buf);
            this->line_info.arg2 = std::stoi(arg.substr(1, arg.length()));

            // Get offset6, which must be a literal
            if(!this->getNextArg())
            {
                this->line_info.error = true;
                break;
            }
            arg = std::string(this->token_buf);
            this->line_info.imm = std::stoi(arg.substr(1, arg.length()));
            this->line_info.is_imm = true;      // redundant?

            this->skipLine();
            break;

        default:
            std::cout << "[" << __FUNCTION__ << "] invalid opoode <" 
                << std::hex << o.opcode << std::endl;
            break;
    }

}


void Lexer::parseDirective(void)
{
    this->line_info.is_directive = true;
    this->line_info.line_num     = this->cur_line;
    this->line_info.opcode.mnemonic     = "DIRECTIVE";
    this->readSymbol();

    // Note: these symbols are also LC3 assembly specific, 
    // in the generic version they will need to be moved out 
    // to some other class / struct
    if(std::string(this->token_buf) == LEX_ORIG)
        this->line_info.symbol = LEX_ORIG;
    else if(std::string(this->token_buf) == LEX_END)
        this->line_info.symbol = LEX_END;
    else if(std::string(this->token_buf) == LEX_BLKW)
        this->line_info.symbol = LEX_BLKW;
    else if(std::string(this->token_buf) == LEX_FILL)
        this->line_info.symbol = LEX_FILL;
    else if(std::string(this->token_buf) == LEX_STRINGZ)
        this->line_info.symbol = LEX_STRINGZ;
    else
        this->line_info.symbol = LEX_INVALID;

    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] (line " << 
            this->cur_line << ") extracted directive symbol " <<
            std::string(this->token_buf) << " and set info.symbol as <"
            << this->line_info.symbol << ">" << std::endl;
    }
    
    if(this->line_info.symbol == LEX_INVALID)
    {
        this->line_info.error = true;
        return;
    }

    if(this->line_info.symbol == LEX_END)
        return;

    // Try to read the arg (if any)
    this->skipWhitespace();
    if(this->getNextArg())
    {
        std::string arg(this->token_buf);
        //this->readSymbol();
        if(this->verbose)
        {
            std::cout << "[" << __FUNCTION__ << 
                "] found directive symbol <" 
                << std::string(this->token_buf) << ">" << std::endl;
        }
        if(this->token_buf[0] == '#')
            this->line_info.imm = std::stoi(arg.substr(1, arg.length()));
        else if(this->token_buf[0] == 'x' || this->token_buf[0] == 'X')
            this->line_info.imm = std::stoi("0" + arg.substr(1, arg.length()));
        else
            this->line_info.imm = std::stoi(arg);
        
        if(this->verbose)
        {
            std::cout << "[" << __FUNCTION__ << "] set imm to <" <<
                this->line_info.imm << ">" << std::endl;
        }

        // If we got am .ORIG directive, set the internal
        // address counter 
        if(this->line_info.symbol == LEX_ORIG)
        {
            this->cur_addr = this->line_info.imm;
        }
    }
}

/*
 * parseLine()
 * Parse a single line of the assembly source
 */
void Lexer::parseLine(void)
{
    // TODO ; may need a GOTO to a block which
    // increments this->cur_addr
    Opcode o;

    initLineInfo(this->line_info);   
    this->line_info.line_num = this->cur_line;
    // Could be a directive 
    if(this->isDirective())
    {
        if(this->verbose)
        {
            std::cout << "[" << __FUNCTION__ << "] (line " << 
                this->cur_line << ") found directive <" << 
                std::string(this->token_buf) << ">" << std::endl;
        }
        this->parseDirective();
        goto LINE_END;
    }

    // Not a directive, load symbol into token_buf and
    // check against known opcodes
    this->readSymbol();
    if(!this->isMnemonic())
    {
        // If not mnemonic then must be label
        this->line_info.is_label = true;
        if(std::string(this->token_buf) == "HALT")
        {
            if(this->verbose)
            {
                std::cout << "[" << __FUNCTION__ << "] got HALT (line " 
                    << this->cur_line << ")" << std::endl;
            }
            this->line_info.opcode.mnemonic = "HALT";
            goto LINE_END;
        }

        if(this->verbose)
        {
            std::cout << "[" << __FUNCTION__ << "] (line " << 
                this->cur_line << ") found label symbol <" 
                << std::string(this->token_buf) << ">" << std::endl;
        }

        this->line_info.label = std::string(this->token_buf);
        //this->advance();        // skip past the ':'
        this->skipWhitespace();

        // We may have a labelled directive
        if(this->isDirective())
        {
            if(this->verbose)
            {
                std::cout << "[" << __FUNCTION__ << "] (line " <<
                    this->cur_line << ") found labelled directive <" << 
                    std::string(this->token_buf) << ">" << std::endl;
            }
            this->parseDirective();
            goto LINE_END;
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
    this->line_info.opcode = o;
    if(o.mnemonic == "M_INVALID")
    {
        if(this->verbose)
        {
            std::cout << "[" << __FUNCTION__ << "] opcode " <<
                std::string(this->token_buf) << " not a valid opcode" << std::endl;
        }
        this->line_info.error = true;
        goto LINE_END;
    }
    this->skipWhitespace();
    this->parseOpcode(o);

LINE_END:
    this->line_info.addr = this->cur_addr;
    this->cur_addr++;
}

// Do lexing pass
SourceInfo Lexer::lex(void)
{
    this->cur_line = 1;
    this->cur_pos  = 0;
    this->cur_addr = 0;

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
            this->parseLine();
            this->source_info.add(this->line_info);
            continue;
        }
    }

    return this->source_info;
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
