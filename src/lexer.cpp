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
    {
        this->cur_line = this->cur_line + 1;
        if(this->verbose)
        {
            std::cout << "[" << __FUNCTION__ << "] advanced to line "
                << std::dec << this->cur_line << std::endl;
        }
    }
}

/*
 * exhausted()
 * Returns true when there is no more input to lex
 */
bool Lexer::exhausted(void) const
{
    return (this->cur_char == '\0' ||
            this->cur_pos >= this->src.size()) ? true : false;
}

/* 
 * skipWhitespace()
 * Eat a sequence of continuous whitespace characters
 */
void Lexer::skipWhitespace(void) 
{
    while(!this->exhausted())
    {
        if(this->cur_char == ' '  || 
           this->cur_char == '\n' || 
           this->cur_char == '\t')
            this->advance();
        else
            break;
    }
}

void Lexer::skipComment(void)
{
    while(this->cur_char != '\n')
        this->advance();
}

void Lexer::skipSeperators(void)
{
    while(!this->exhausted())
    {
        if(this->cur_char == ',' ||
           this->cur_char == ':' ||
           this->cur_char == ';')
            this->advance();
        else
            break;
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
    return (this->cur_char == '.' || this->token_buf[0] == '.');
}
bool Lexer::isSpace(void)
{
    return (this->cur_char == ' '  || 
            this->cur_char == '\t' ||
            this->cur_char == '\n') ? true : false;
}
bool Lexer::isComment(void)
{
    return (this->cur_char == ';');
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
/*
 * isTrapOp
 * Determine if the current opcode is a valid TRAP 
 * psuedo op
 */
bool Lexer::isTrapOp(void)
{
    if(this->token_buf[0] == '\0')
        return false;
    Opcode op;
    this->psuedo_op_table.get(std::string(this->token_buf), op);

    return (op.mnemonic != "M_INVALID") ? true : false;
}

/*
 * isValidArg()
 * Returns true if the token buffer contents can be interpreted as a 
 * valid argument
 */
bool Lexer::isValidArg(void)
{
    return (this->token_buf[0] == 'r' ||
            this->token_buf[0] == 'R' || 
            this->token_buf[0] == '#' ||
            this->token_buf[0] == 'x' ||
            this->token_buf[0] == 'X') ? true : false;
}

void Lexer::skipLine(void)
{
    while(this->cur_char != '\n')
        this->advance();
    // skip ahead over newline
    this->advance();
}

/*
 * scanToken()
 * Scan a complete token into the token buffer
 */
void Lexer::scanToken(void)
{
    unsigned int idx = 0;
    this->skipWhitespace();     // eat any leading whitespace 
    this->skipSeperators();     // eat any seperators that might be left
    while(idx < (this->token_buf_size-1))
    {
        if(this->cur_char == ' ')       // end 
            break;
        if(this->cur_char == '\n')      // newline
            break;
        if(this->cur_char == ';')       // comment
            break;
        if(this->cur_char == ',')       // seperator
            break;
        this->token_buf[idx] = this->cur_char;
        this->advance();
        idx++;
    }
    this->token_buf[idx] = '\0';
    // If we are on a seperator now, advance the source pointer 
    if(this->cur_char == ',')
        this->advance();

    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] (line " << std::dec << 
            this->cur_line << ") : token_buf contains <" << 
            std::string(this->token_buf) << "> " << std::endl;
    }
}

/*
 * parseOpcode3Args()
 * Parse operands to an LC3 assembly language opcode
 */
void Lexer::parseOpcode3Args(void)
{
    int err_argnum = 0;
    bool arg_err = false;
    std::string arg;
    // Get arg1
    this->scanToken();
    if(!this->isValidArg())
    {
        err_argnum = 1;
        arg_err = true;
        goto ARG_ERR;
    }
    arg = std::string(this->token_buf);
    this->line_info.arg1 = std::stoi(arg.substr(1, arg.length()));

    // Get source 1
    this->scanToken();
    if(!this->isValidArg())
    {
        err_argnum = 2;
        arg_err = true;
        goto ARG_ERR;
    }
    arg = std::string(this->token_buf);
    this->line_info.arg2 = std::stoi(arg.substr(1, arg.length()));

    // Get source 2
    this->scanToken();
    if(!this->isValidArg())
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
                std::dec << this->cur_line << ") error parsing arg " <<
                err_argnum << " of " << this->line_info.opcode.mnemonic << 
                " opcode" << std::endl;
        }
        this->line_info.error = true;
        this->line_info.errstr = "Argument " + std::to_string(err_argnum) + " of opcode " + this->line_info.opcode.mnemonic + " invalid";
    }
}

/* 
 * parseOpcode
 * Parse an opcode in the LC3 assembly language 
 */
void Lexer::parseOpcode(void)
{
    Opcode o;
    this->op_table.get(std::string(this->token_buf), o);
    this->line_info.opcode = o;
    if(o.mnemonic == "M_INVALID")
    {
        this->line_info.error = true;
        this->line_info.errstr = "Invalid opcode <" + std::string(this->token_buf) + ">";
        if(this->verbose)
        {
            std::cout << "[" << __FUNCTION__ << "] " << this->line_info.errstr << std::endl;
        }
        return;
    }

    if(this->verbose)
        std::cout << "[" << __FUNCTION__ << "] decoding <" 
            << o.mnemonic << ">" << std::endl;
    
    std::string arg;
    switch(o.opcode)
    {
        case LC3_ADD:
            // 3 args, comma seperated (DST, SR1, SR2)
            this->parseOpcode3Args();
            break;

        case LC3_AND:
            // 3 args, comma seperated (DST, SR1, SR2)
            this->parseOpcode3Args();
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
            this->scanToken();
            if(this->isValidArg())      // assume literal
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
                this->scanToken();
                if(!this->isValidArg())
                {
                    this->line_info.error = true;
                    this->line_info.errstr = "Failed to parse JMP argument <" + std::string(this->token_buf) + ">";
                    break;
                }
                arg = std::string(this->token_buf);
                this->line_info.arg2 = std::stoi(arg.substr(1, arg.length()));
            }
            else if(o.mnemonic == "RET")
            {
                this->line_info.arg1 = 0x0;
                this->line_info.arg2 = 0x3;
            }
            else
            {
                this->line_info.error = true;
                this->line_info.errstr = "Invalid jump opcode " + o.mnemonic;
                if(this->verbose)
                {
                    std::cout << "[" << __FUNCTION__ << 
                        "] " << this->line_info.errstr << std::endl;
                }
            }
            break;

        case LC3_JSR:
            this->scanToken();
            if(!this->isValidArg())
            {
                this->line_info.error = true;
                this->line_info.errstr = "Failed to parse JSR immediate <" + std::string(this->token_buf) + ">";
                break;
            }
            arg = std::string(this->token_buf);
            if(o.mnemonic == "JSR")
            {
                this->line_info.arg1 = 0x4;
                this->line_info.imm = std::stoi(arg.substr(1, arg.length()));
            }
            else if(o.mnemonic == "JSRR")
            {
                this->line_info.arg1 = 0x0;
                this->line_info.arg2 = std::stoi(arg.substr(1,arg.length()));
            }
            else
            {
                this->line_info.error = true;
                this->line_info.errstr = "Invalid jump opcode " + o.mnemonic; 
                if(this->verbose)
                {
                    std::cout << "[" << __FUNCTION__ << 
                        "] " << this->line_info.errstr << std::endl;
                }
            }
            break;

        case LC3_LEA:
            this->scanToken();
            if(!this->isValidArg())
            {
                this->line_info.error = true;
                this->line_info.errstr = "Failed to parse LEA argument <" + std::string(this->token_buf) + ">";
                break;
            }
            arg = std::string(this->token_buf);
            this->line_info.arg1 = std::stoi(arg.substr(1, arg.length()));
            // Next could be a symbol
            if(this->isNumber())
            {
                this->scanToken();
                this->line_info.imm = std::stoi(std::string(this->token_buf));
            }
            else
            {
                this->scanToken();
                this->line_info.symbol = std::string(this->token_buf);
            }

            break;

        // These have a similar offset structure 
        case LC3_LD:
        case LC3_LDI:
            this->scanToken();
            if(!this->isValidArg())
            {
                this->line_info.error = true;
                this->line_info.errstr = "Failed to parse LD argument <" + std::string(this->token_buf) + ">"; 
                break;
            }
            arg = std::string(this->token_buf);
            this->line_info.arg1 = std::stoi(arg.substr(1, arg.length()));
            // Next could be a symbol
            if(this->isNumber())
            {
                this->scanToken();
                this->line_info.imm = std::stoi(std::string(this->token_buf));
            }
            else
            {
                this->scanToken();
                this->line_info.symbol = std::string(this->token_buf);
            }
            break;

        case LC3_LDR:
            // Get arg1 register 
            this->scanToken();
            if(!this->isValidArg())
            {
                this->line_info.error = true;
                this->line_info.errstr = "Failed to parse LDR argument <" + std::string(this->token_buf) + "> (destination register)";
                break;
            }
            arg = std::string(this->token_buf);
            this->line_info.arg1 = std::stoi(arg.substr(1, arg.length()));
            // Get base register 
            this->scanToken();
            if(!this->isValidArg())
            {
                this->line_info.error = true;
                this->line_info.errstr = "Failed to parse LDR argument <" + std::string(this->token_buf) + "> (base register)";
                break;
            }
            arg = std::string(this->token_buf);
            this->line_info.arg2 = std::stoi(arg.substr(1, arg.length()));

            // Get offset6, which must be a literal
            this->scanToken();
            if(!this->isValidArg())
            {
                this->line_info.error = true;
                this->line_info.errstr = "Failed to parse LDR argument <" + std::string(this->token_buf) + "> (offset6)"; 
                break;
            }
            arg = std::string(this->token_buf);
            this->line_info.imm = std::stoi(arg.substr(1, arg.length()));
            this->line_info.is_imm = true;      // redundant?
            
            break;

        case LC3_NOT:
            // Get dest 
            this->scanToken();
            if(!this->isValidArg())
            {
                this->line_info.error = true;
                this->line_info.errstr = "Failed to parse NOT argument <" + std::string(this->token_buf) + "> (destination register)"; 
                break;
            }
            arg = std::string(this->token_buf);
            this->line_info.arg1 = std::stoi(arg.substr(1, arg.length()));
            // Get dst 
            this->scanToken();
            if(!this->isValidArg())
            {
                this->line_info.error = true;
                this->line_info.errstr = "Failed to parse NOT argument <" + std::string(this->token_buf) + "> (source register)"; 
                break;
            }
            arg = std::string(this->token_buf);
            this->line_info.arg2 = std::stoi(arg.substr(1, arg.length()));
            break;

        // These have a similar opcode structure 
        case LC3_ST:
        case LC3_STR:
            // Get src register 
            this->scanToken();
            if(!this->isValidArg())
            {
                this->line_info.error = true;
                this->line_info.errstr = "Failed to parse ST argument <" + std::string(this->token_buf) + "> (source register)"; 
                break;
            }
            arg = std::string(this->token_buf);
            this->line_info.arg1 = std::stoi(arg.substr(1, arg.length()));
            // Get base register 
            this->scanToken();
            if(!this->isValidArg())
            {
                this->line_info.error = true;
                this->line_info.errstr = "Failed to parse ST argument <" + std::string(this->token_buf) + "> (base register)"; 
                break;
            }
            arg = std::string(this->token_buf);
            this->line_info.arg2 = std::stoi(arg.substr(1, arg.length()));

            // Get offset6, which must be a literal
            this->scanToken();
            if(!this->isValidArg())
            {
                this->line_info.error = true;
                this->line_info.errstr = "Failed to parse ST argument <" + std::string(this->token_buf) + "> (offset6)"; 
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
    this->line_info.opcode.mnemonic = "TRAP";
    this->line_info.opcode.opcode   = LC3_TRAP;

    std::string tok = std::string(this->token_buf);
    this->psuedo_op_table.get(std::string(this->token_buf), o);
    if(o.mnemonic == "M_INVALID")
    {
        this->line_info.error = true;
        this->line_info.errstr = "Not a valid TRAP psuedo-op <" + std::string(this->token_buf) + ">";
        if(this->verbose)
        {
            std::cout << "[" << __FUNCTION__ << "] " <<
                this->line_info.errstr << std::endl;
        }
        return;
    }

    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] (line " << std::dec << 
            this->cur_line << ") parsing TRAP opcode <0x" 
            << std::hex << this->line_info.opcode.opcode << "> " << std::endl;
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
            this->line_info.errstr = "No implementation for opcode <" 
                + tok;
            if(this->verbose)
            {
                std::cout << "[" << __FUNCTION__ << "] " <<
                    this->line_info.errstr << std::endl;
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
    this->line_info.is_directive  = true;
    this->asm_dir_table.get(std::string(this->token_buf), o);
    if(o.mnemonic == "M_INVALID" || o.mnemonic == ".INVALID")
    {

        this->line_info.error = true;
        this->line_info.errstr = "Opcode <" + std::string(this->token_buf) + "> not a valid assembler directive";
        if(this->verbose)
        {
            std::cout << "[" << __FUNCTION__ << "] " << 
                this->line_info.errstr << std::endl;
        }
        return;
    }
    this->line_info.opcode = o;
    this->line_info.opcode.opcode = 0x0;    // zero out opcode for directives
    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] (line " << 
            std::dec << this->cur_line << ") extracted directive symbol " <<
            std::string(this->token_buf) << std::endl;
    }

    if(o.mnemonic == ".END")
        return;

    // Remaining directives have arguments, figure those out here 
    this->scanToken();
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
            this->cur_addr = arg-1; // will be incremented at end of line
            this->line_info.imm = arg;
            break;
        case ASM_STRINGZ:
            std::cout << "[" << __FUNCTION__ << 
                "] .STRINGZ not yet implemented" << std::endl;
            break;
        default:
            this->line_info.error = true;
            this->line_info.errstr = "Token <" + o.mnemonic + "> is not a valid assembler directive";
            if(this->verbose)
            {
                std::cout << "[" << __FUNCTION__ << "] " << 
                    this->line_info.errstr << std::endl;
            }
            break;
    }
}

/*
 * parseToken()
 * Deduce the meaning of a single token, calling on more
 * specialized functions to parse as meaning is interpreted
 */
void Lexer::parseToken(void)
{
    this->scanToken();
    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] scanned " << 
            std::string(this->token_buf) << " into token buffer\n";
    }

    // Check if token is a directive
    if(this->isDirective())
    {
        if(this->verbose)
        {
            std::cout << "[" << __FUNCTION__ << "] (line " << 
                std::dec << this->cur_line << ") found directive <" << 
                std::string(this->token_buf) << ">" << std::endl;
        }
        this->parseDirective();
        return;
    }

    // Check if token is a TRAP psuedo-op
    if(this->isTrapOp())
    {
        if(this->verbose)
        {
            std::cout << "[" << __FUNCTION__ << "] (line " << 
                std::dec << this->cur_line << ") found trap opcode <" << 
                std::string(this->token_buf) << ">" << std::endl;
        }
        this->parseTrapOpcode();
        return;
    }

    // Could be a valid mnemonic
    if(this->isMnemonic())
    {
        if(this->verbose)
        {
            std::cout << "[" << __FUNCTION__ << "] (line " << 
                std::dec << this->cur_line << ") found opcode <" << 
                std::string(this->token_buf) << ">" << std::endl;
        }
        this->parseOpcode();
        return;
    }

    // If we get here and the label status is set, then we 
    // previously found a label but cannot find a corresponding
    // instruction
    if(this->line_info.is_label)
    {
        this->line_info.error = true;
        this->line_info.errstr = "No valid instruction after label <" + 
            this->line_info.label + ">";
        std::cerr << "[" << __FUNCTION__ << "] (line " << 
            std::dec << this->line_info.line_num << ") " << 
            this->line_info.errstr << std::endl;
        return;
    }

    // Since its neither an opcode or a psuedo op, we assume
    // its a label and store into symbol table
    this->line_info.is_label = true;
    if(this->verbose)
    {
        std::cout << "[" << __FUNCTION__ << "] (line "
            << std::dec << this->cur_line << ") found label symbol <" 
            << std::string(this->token_buf) << "> at address 0x"
            << std::hex << std::setw(4) << std::setfill('0') 
            << this->cur_addr << std::endl;
    }
        
    // add the label, removing any trailing characters (eg ':')
    std::string label(this->token_buf);
    // Ensure that this actually turned into a valid token 
    if(label.length() == 0)
    {
        this->line_info.error = true;
        this->line_info.errstr = "(line " + std::to_string(this->line_info.line_num) + ") Invalid label token";
        if(this->verbose)
            std::cout << "[" << __FUNCTION__ << "] " << this->line_info.errstr << std::endl;
        return;
    }
    if(label[label.length()-1] == ':')
        this->line_info.label = label.substr(0, label.length()-1);
    else
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
    //this->skipWhitespace();
}

/*
 * parseLine()
 * Parse a single line of the assembly source
 */
void Lexer::parseLine(void)
{
    initLineInfo(this->line_info);   
    this->line_info.line_num = this->cur_line;

    this->parseToken();
    // Check the line info to see what kind of line it was. 
    if(this->line_info.error)
    {
        std::cerr << "[" << __FUNCTION__ << "] error line ("
            << std::dec << this->line_info.line_num << ") "
            << this->line_info.errstr << std::endl;
        goto LINE_END;
    }

    // If it was a label, we need to call parseToken() again 
    if(this->line_info.is_label)
    {
        if(this->verbose)
        { 
            std::cout << "[" << __FUNCTION__ << "] got label on line "
                << std::dec << this->line_info.line_num << std::endl;
            std::cout << "[" << __FUNCTION__ << "] parsing segment after label" << std::endl;
        }
        this->skipWhitespace();
        this->parseToken();
    }

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
            this->skipLine();
            continue;
        }

        this->parseLine();
        this->source_info.add(this->line_info);
        if(this->line_info.error)
        {
            std::cout << "[" << __FUNCTION__ << "] (line " << 
                this->line_info.line_num << ") ERROR " << 
                this->line_info.errstr << std::endl;
            this->source_info.setError(true);
            return this->source_info;
        }
    }
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
        std::cout << "[" << __FUNCTION__ << "] read " 
        << std::dec << this->src.length() << " characters from file ["
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
