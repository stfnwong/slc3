/* LEXER
 * Lexer object for scanning assembly files and extracting
 * tokens.
 * 
 * Stefan Wong 2018
 */

#ifndef __LEXER_HPP
#define __LEXER_HPP

#include <string>

// Some lexer contants
typedef enum {
    LEX_UNK, LEX_EOF, LEX_EOL, LEX_ID, LEX_NUM, 
    LEX_COM, LEX_COMMA, LEX_PLUS, LEX_MINUS, LEX_STAR
} LexSym;

class Token
{
    public:
        LexSym type;
        std::string lexeme;
        int num;
        bool label;

    public:
        Token(const LexSym t, const std::string& lex, const int v, const bool l);
        ~Token();
        Token(const Token& that);
};

class Lexer
{
    private:
        std::string filename;
        std::string src;
        unsigned int pos;
        char cur_char;
        char* token_buf;        // build up token here
        uint32_t buf_size;
        void allocMem(void);
        // Source movement
        void advance(void);
        // Token handlers 
        bool isDigit(void);
        bool isAlpha(void);
        bool isSym(void);

        //Token getNextToken(void);
        void getNextToken(Token& t);
        // As per Lexer in "Compilers and Compiler generators"
        void getComment(void);

    public:
        Lexer();
        Lexer(const std::string& filename);
        ~Lexer();
        Lexer(const Lexer& that) = delete;

        // Load source from disk
        void loadFile(const std::string& filename);
        
        // Manipulate characters in source 
        bool endline(void);
        char nextChar(void);
        char getChar(void) const;

        // Standard getters 
        unsigned int getSrcLength(void) const;
        std::string getFilename(void) const;
        std::string dumpSrc(void) const;

};


#endif /*__LEXER_HPP*/
