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

// Largest size allowable for a token
#define LEX_TOKEN_MAX_LEN 64

// Some lexer contants
typedef enum {
    LEX_UNK, LEX_EOF, LEX_EOL, LEX_ID, LEX_NUM, 
    LEX_COM, LEX_COMMA, LEX_PLUS, LEX_MINUS, LEX_STAR
} LexSym;

// NOTE: This is a LC3 specific lineinfo
// structure. Consider generalizing in
// future
typedef struct{
    unsigned int line_num;
    unsigned int addr;
    std::string  symbol;     //char* symbol // <- faster?;
    std::string  label;     //char* label // <- faster?;
    // Opcode / Operand info
    Opcode       opcode;
    uint16_t     dest;
    uint16_t     src1;
    uint16_t     src2;
    uint16_t     imm;
    bool         is_label;
    bool         is_directive;
    bool         error;
} LineInfo;

void initLineInfo(LineInfo& l);

// Assembler directives 
const char LEX_ORIG[] = ".ORIG";
const char LEX_END[]  = ".END";
const char LEX_BLKW[] = ".BLKW";
const char LEX_FILL[] = ".FILL";
const char LEX_STRINGZ[] = ".STRINGZ";
const char LEX_INVALID[] = ".INVALID";

class SourceInfo
{
    private:
        std::vector <LineInfo> line_info;
        
    public:
        SourceInfo();
        ~SourceInfo();
        // Add/remove lines
        void add(const LineInfo& l);
        LineInfo get(const unsigned int idx) const;
        unsigned int getLineNum(const unsigned int idx) const;
}; 

typedef struct
{
    uint16_t addr;
    std::string name;
} Symbol;

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

    private:
        // Source movement
        void advance(void);
        bool exhausted(void) const;
        void skipWhitespace(void);
        void skipComment(void);
        void readSymbol(void);
        bool isSymbol(void);
        bool isSpace(void);
        bool isMnemonic(void);
        // TODO: may only need this during debug 
        void skipLine(void);
        
    private:
        // Symbol parse
        LineInfo parseDirective(void);
        LineInfo parseSymbol(void);
        LineInfo parseLabelSymbol(void);
        void     parseNextArg(void);

    // Source internals 
    private:
        unsigned int cur_line;
    public:
        // Lexing function
        void lex(void);

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

        // TODO : debug, remove 
        bool isASCII(void) const;
        char dumpchar(const unsigned int idx) const;

        OpcodeTable dumpOpTable(void) const;
        unsigned int opTableSize(void) const;
        SourceInfo  dumpSrcInfo(void) const;
};

#endif /*__LEXER_HPP*/
