/* LEXER
 * Lexer object for scanning assembly files and extracting
 * tokens.
 * 
 * Stefan Wong 2018
 */

#include <iostream>
#include <fstream>
#include <cstdlib>  // for std::atoi()
#include "lexer.hpp"

/*
 * TOKEN 
 */
Token::Token(const LexSym t, const std::string& lex, const int val, const bool l)
{
    this->type   = t;
    this->lexeme = lex;
    this->num    = val;
    this->label  = l;
}

Token::~Token() {} 
// copy ctor
Token::Token(const Token& that)
{
    this->type   = that.type;
    this->lexeme = that.lexeme;
    this->num    = that.num;
    this->label  = that.label;
}
 

/*
 * LEXER 
 */
Lexer::Lexer()
{
    this->pos = 0;
    this->buf_size = 4096;
    this->allocMem();
}

Lexer::Lexer(const std::string& filename)
{
    this->loadFile(filename);
    this->pos = 0;
    this->buf_size = 4096;
    this->allocMem();
}

Lexer::~Lexer()
{
    delete[] this->token_buf;
}

void Lexer::allocMem(void)
{
    this->token_buf = new char[this->buf_size];
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
    this->pos++;
    if(this->pos > this->src.size())
        this->cur_char = '\0';
    else
        this->cur_char = this->src[this->pos];
}

bool Lexer::isDigit(void)
{

}
bool Lexer::isAlpha(void)
{

}

//Token Lexer::getNextToken(void) {}

void Lexer::getNextToken(Token& t) {}

void Lexer::getComment(void)
{
    unsigned int length = 0;
    while(!this->endline())
    {
        if(length < this->buf_size)
        {
            this->token_buf[length] = this->cur_char;
            length++;
        }
        this->advance();
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
}

// ==== CHARACTER MANIPULATION


bool Lexer::endline(void)
{
    this->advance();
    return (this->cur_char == '\n');
}

// TODO: maybe nix this method
char Lexer::nextChar(void)
{
    this->advance();
    return this->cur_char;
    //this->cur_char = this->src[pos];
    //if(this->cur_char == '\0')
    //    return this->cur_char;      // Input exhausted
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
