/* LEXER
 * Lexer object for scanning assembly files and extracting
 * tokens.
 * 
 * Stefan Wong 2018
 */

#include <iostream>
#include <iomanip>
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

    // TODO : Note that this is using the 'global' vars
    // which is not great. Will need to be refactored
    for(const Opcode &op : lc3_psuedo_op_list)
        this->psuedo_op_table.add(op);
    for(const Opcode &op : LEX_ASM_DIRECTIVE_OPCODES)
        this->asm_dir_table.add(op);
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

bool Lexer::isTrapOp(void)
{
    if(this->token_buf[0] == '\0')
        return false;
    Opcode op;
    this->psuedo_op_table.get(std::string(this->token_buf), op);

    return (op.mnemonic != "M_INVALID") ? true : false;
}

void Lexer::skipLine(void)
{
    while(this->cur_char != '\n')
        this->advance();
    // skip ahead one
    this->advance();
}

/*
 * getNextArg()
 * Reads the next complete token into the token buffer
 * and checks if that token consistitues a valid operand
 * for an opcode. Return true if the token buffer contents
 * are a valid operand, false otherwise.
 */
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

/*
 * parseOpcodeArgs()
 * Parse operands to an LC3 assembly language opcode
 */
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

/* 
 * parseOpcode
 * Parse an opcode in the LC3 assembly language 
 */
void Lexer::parseOpcode(const Opcode& o)
{
    // TODO : How to generically parse Opcodes?  Maybe I need virtual 
    // methods here. This won't be such a problem until I decide to 
    // 'genericize' the assembler
    if(this->verbose)
        std::cout << "[" << __FUNCTION__ << "] decoding <" 
            << o.mnemonic << ">" << std::endl;
    
    std::string arg;
    switch(o.opcode)
    {
        case LC3_ADD:
            // 3 args, comma seperated (DST, SR1, SR2)
            this->parseOpcodeArgs();
            break;

        case LC3_AND:
            // 3 args, comma seperated (DST, SR1, SR2)
            this->parseOpcodeArgs();
            break;

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

            break;

        case LC3_JMP_RET:
            // Decide if this is JMP or RET
            if(o.mnemonic == "JMP")
            {
            }
            else if(o.mnemonic == "RET")
            {
            }
            else
            {
                this->line_info.error = true;
                if(this->verbose)
                {
                    std::cout << "[" << __FUNCTION__ << 
                        "] invalid jump opcode " << o.mnemonic 
                        << std::endl;
                }
            }

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

            break;

        // These have a similar offset structure 
        case LC3_LD:
        case LC3_LDI:
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
            
            break;

        case LC3_NOT:
            // Get src 
            if(!this->getNextArg())
            {
                this->line_info.error = true;
                break;
            }
            arg = std::string(this->token_buf);
            this->line_info.arg1 = std::stoi(arg.substr(1, arg.length()));
            // Get dst 
            if(!this->getNextArg())
            {
                this->line_info.error = true;
                break;
            }
            arg = std::string(this->token_buf);
            this->line_info.arg2 = std::stoi(arg.substr(1, arg.length()));
            break;

        // These have a similar opcode structure 
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

            break;

        default:
            std::cout << "[" << __FUNCTION__ << "] invalid opoode <" 
                << std::hex << o.opcode << std::endl;
            break;
    }
}

/*
 * parseTrapOpcode
 * Parse an assembly directive that corresponds to a trap opcode
 */
void Lexer::parseTrapOpcode(void)
{
    Opcode o;

    this->line_info.is_directive    = false;
    this->line_info.line_num        = this->cur_line;
    this->line_info.opcode.mnemonic = "TRAP";
    this->line_info.opcode.opcode   = LC3_TRAP;

    std::string tok = std::string(this->token_buf);
    this->psuedo_op_table.get(std::string(this->token_buf), o);
    if(o.mnemonic == "M_INVALID")
    {
        if(this->verbose)
        {
            std::cout << "[" << __FUNCTION__ << "] opcode " <<
                std::string(this->token_buf) << " not a valid trap opcode" << std::endl;
        }
        this->line_info.error = true;
        return;
    }

    // TODO : These are also 'hardcoded' for now. We want to 
    // eventually move them out so that some kind of LC3 specific
    // assembler can be specialized out of generic parts
    switch(o.opcode)
    {
        case LC3_GETC:
            this->line_info.imm = 0x20;
            break;

        case LC3_OUT:
            this->line_info.imm = 0x21;
            break;

        case LC3_PUTS:
            this->line_info.imm = 0x22;
            break;

        case LC3_IN:
            this->line_info.imm = 0x23;
            break;

        case LC3_PUTSP:
            this->line_info.imm = 0x24;
            break;

        case LC3_HALT:
            this->line_info.imm = 0x25;
            break;

        default:
            this->line_info.error = true;
            if(this->verbose)
            {
                std::cout << "[" << __FUNCTION__ << 
                    "] no implementation for trap opcode " <<
                    o.mnemonic << std::endl;
            }
            break;
    }
}

/*
 * parseDirective()
 * Parse an assembly directive
 */
void Lexer::parseDirective(void)
{
    Opcode o;
    this->line_info.is_directive    = true;
    this->line_info.line_num        = this->cur_line;
    ///this->line_info.opcode.mnemonic = "<DIRECTIVE>";
    this->readSymbol();

    // TODO: these symbols are also LC3 assembly specific, 
    // in the generic version they will need to be moved out 
    // to some other class / struct
    // TODO : move these into another OpcodeTable structure for
    // assembly directive.

    this->asm_dir_table.get(std::string(this->token_buf), o);
    if(o.mnemonic == "M_INVALID" || o.mnemonic == ".INVALID")
    {
        if(this->verbose)
        {
            std::cout << "[" << __FUNCTION__ << "] opcode " <<
                std::string(this->token_buf) << " not a valid assembler directive opcode" << std::endl;
        }
        this->line_info.error = true;
        return;
    }
    this->line_info.opcode = o;

    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] (line " << 
            this->cur_line << ") extracted directive symbol " <<
            std::string(this->token_buf) << std::endl;
    }

    if(o.mnemonic == ".END")
        return;

    // Remaining directives have arguments, figure those out here 
    this->skipWhitespace();
    this->getNextArg();
    // Get the arg in numerical form from string
    uint16_t arg = 0;
    std::string argstr = std::string(this->token_buf);
    if(this->token_buf[0] == 'x' || this->token_buf[0] == 'X')
        arg = std::stoi(argstr.substr(1, argstr.length()), nullptr, 16);
    else if(this->token_buf[0] == '#')
        arg = std::stoi(argstr.substr(1, argstr.length()));
    else
        arg = std::stoi(argstr);
    switch(o.opcode)
    {
        case ASM_BLKW:
            // Insert memory in place
            this->line_info.imm = arg;
            this->cur_addr += arg;
            break;
        case ASM_FILL:
            this->line_info.imm = arg;
            break;
        case ASM_ORIG:
            this->cur_addr = arg;
            this->line_info.imm = arg;
            break;

        case ASM_STRINGZ:
            std::cout << "[" << __FUNCTION__ << 
                "] .STRINGZ not yet implemented" << std::endl;
            break;

        default:
            this->line_info.error = true;
            break;
    }
}

/*
 * parseLine()
 * Parse a single line of the assembly source
 */
void Lexer::parseLine(void)
{
    Opcode o;

    initLineInfo(this->line_info);   
    this->line_info.line_num = this->cur_line;

    // Could be an unlabelled directive 
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
        this->line_info.is_label = true;
        // Check for psuedo ops associated with TRAP vectors 
        if(this->isTrapOp())
        {
            this->parseTrapOpcode();
            goto LINE_END;
        }

        // Since its neither an opcode or a psuedo op, we assume
        // its a label and store into symbol table
        if(this->verbose)
        {
            std::cout << "[" << __FUNCTION__ << "] (line " << 
                this->cur_line << ") found label symbol <" 
                << std::string(this->token_buf) << "> at address 0x"
                << std::hex << std::setw(4) << std::setfill('0') 
                << this->cur_addr << std::endl;
        }

        // Add the address of this symbol to symbol table
        this->line_info.label = std::string(this->token_buf);
        Symbol s;
        // Get rid of any trailing non-alphanum chars 
        std::string sym_label = std::string(this->token_buf);
        while(!isalnum(toupper(sym_label[sym_label.length()-1])))
            sym_label.pop_back();
        s.label = sym_label;
        s.addr  = this->cur_addr;
        std::cout << "s.label [" << s.label <<  "] s.addr : " << std::hex << s.addr << std::endl;
        this->sym_table.add(s);
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

/*
 * resolveLabels9)
 * Replace labels with their corresponding addresses
 */
void Lexer::resolveLabels(void)
{
    LineInfo info;
    unsigned int n;
    uint16_t label_addr;

    for(n = 0; n < this->source_info.getNumLines(); n++)
    {
        info = this->source_info.get(n);
        //if(!info.is_label && info.label != "\0")
        if(info.symbol != "\0" && !info.is_directive)
        {
            label_addr = this->sym_table.getAddr(info.symbol);
            if(this->verbose)
            {
                std::cout << "[" << __FUNCTION__ << 
                    "] resolving symbol " << info.symbol << 
                    " which has address " << std::hex << label_addr
                    << std::endl;
            }
            if(label_addr > 0)
            {
                // TODO : need to know what kind of opcode
                // it was to know where to put the address
                info.imm = label_addr;
                this->source_info.update(n, info);
            }
        }
    }
}

// Do lexing pass
SourceInfo Lexer::lex(void)
{
    this->cur_line = 1;
    this->cur_pos  = 0;
    this->cur_addr = 0;

    // First pass 
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

    std::cout << "DEBUG : dumping symbols before labels are resolved " << std::endl;
    this->sym_table.dump();

    // Second pass 
    this->resolveLabels();

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

    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] read " << 
            this->src.length() << " characters from file ["
            << filename << "]" << std::endl;
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

SymbolTable Lexer::dumpSymTable(void) const
{
    return this->sym_table;
}

#ifdef LEX_DEBUG 
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
#endif /*LEX_DEBUG*/
