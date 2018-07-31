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
#define LEX_TOKEN_MAX_LEN 64

// Assembler directives 
const char LEX_ORIG[]    = ".ORIG";
const char LEX_END[]     = ".END";
const char LEX_BLKW[]    = ".BLKW";
const char LEX_FILL[]    = ".FILL";
const char LEX_STRINGZ[] = ".STRINGZ";
const char LEX_INVALID[] = ".INVALID";

// TODO: for now we just lex for LC3, but maybe we pass
// a machine object here and extract the correct symbol/address
// mappings from that object
class Lexer
{
    private:
        bool verbose;
        OpcodeTable op_table;
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
        void skipLine(void);
        
    private:
        // Symbol parse
        bool getNextArg(void);
        void parseOpcodeArgs(void);
        void parseOpcode(const Opcode& o);
        void parseDirective(void);
        void parseLine(void);

    // Source internals 
    private:
        SymbolTable sym_table;
        unsigned int cur_line;
        void resolveLabels(void);
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
