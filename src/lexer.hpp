/* LEXER
 * Lexer object for scanning assembly files and extracting
 * tokens.
 * 
 * Stefan Wong 2018
 */

#ifndef __LEXER_HPP
#define __LEXER_HPP

#include <cstdint>
#include <string>
#include <vector>
// Pass an opcode table for the machine to the lexer
#include "opcode.hpp"
#include "source.hpp"

#define LEX_DEBUG 
// Largest size allowable for a token
#define LEX_TOKEN_MAX_LEN 32

// Assembler directives (which don't map to trap opcodes)
#define ASM_INVALID 0x00
#define ASM_BLKW    0x01
#define ASM_END     0x02
#define ASM_FILL    0x03
#define ASM_ORIG    0x04
#define ASM_STRINGZ 0x05

//typedef enum {
//    ASM_INVALID, ASM_BLKW, ASM_END, ASM_FILL, ASM_ORIG, ASM_STRINGZ
//} LEX_ASM_DIR;

// Assembler directives that are not trap vectors 
static Opcode LEX_ASM_DIRECTIVE_OPCODES[] = {
    {ASM_BLKW,    ".BLKW"},
    {ASM_END,     ".END"},
    {ASM_FILL,    ".FILL"},
    {ASM_ORIG,    ".ORIG"},
    {ASM_STRINGZ, ".STRINGZ"},
    {ASM_INVALID, ".INVALID"},
};

// TODO: for now we just lex for LC3, but maybe we pass
// a machine object here and extract the correct symbol/address
// mappings from that object
class Lexer
{
    private:
        bool verbose;
        OpcodeTable op_table;
        OpcodeTable psuedo_op_table;
        OpcodeTable asm_dir_table;
        std::string filename;
        std::string src;
        unsigned int cur_pos;
        char cur_char;
        char* token_buf;        // build up token here
        unsigned int token_buf_size;
        void allocMem(void);
        void initVars(void);

    private:
        LineInfo   line_info;       // this might go...
        SourceInfo source_info;
        unsigned int cur_addr;

    private:
        // Source movement
        void advance(void);
        bool exhausted(void) const;
        void skipWhitespace(void);
        void skipComment(void);
        void readSymbol(void);
        bool isSymbol(void) const;
        bool isNumber(void) const;
        bool isDirective(void) const;
        bool isSpace(void);
        bool isMnemonic(void);
        bool isTrapOp(void);
        void skipLine(void);
        
    private:
        // Symbol parse
        bool getNextArg(void);
        void parseOpcodeArgs(void);
        void parseOpcode(const Opcode& o);
        void parseTrapOpcode(void);
        void parseDirective(void);
        void parseLine(void);

    // Source internals 
    private:
        SymbolTable  sym_table;
        unsigned int cur_line;
        void         resolveLabels(void);
    public:
        // Lexing function
        SourceInfo lex(void);

    public:
        Lexer(const OpcodeTable& ot);
        Lexer(const OpcodeTable& ot, const std::string& filename);
        ~Lexer();
        Lexer(const Lexer& that) = delete;

        // Load source from disk
        void loadFile(const std::string& filename);
        // Standard getters 
        unsigned int getSrcLength(void) const;
        std::string getFilename(void) const;
        std::string dumpSrc(void) const;

        void setVerbose(const bool b);
        bool getVerbose(void) const;
        // Dump internal info - remove 
        OpcodeTable dumpOpTable(void) const;
        unsigned int opTableSize(void) const;
        SourceInfo  dumpSrcInfo(void) const;
        SymbolTable dumpSymTable(void) const;
#ifdef LEX_DEBUG 
        bool isASCII(void) const;
        char dumpchar(const unsigned int idx) const;
#endif /*LEX_DEBUG*/
};

#endif /*__LEXER_HPP*/
