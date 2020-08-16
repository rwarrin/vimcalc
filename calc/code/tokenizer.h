#ifndef CALC_TOKENIZER_H

enum token_type
{
    TokenType_Unknown = 0,

    TokenType_OpenParen,
    TokenType_CloseParen,
    TokenType_Asterisk,
    TokenType_Minus,
    TokenType_Plus,
    TokenType_ForwardSlash,
    TokenType_Percent,
    TokenType_Pipe,
    TokenType_Ampersand,
    TokenType_Carat,
    TokenType_Tilde,
    TokenType_LessThan,
    TokenType_GreaterThan,

    TokenType_Equals,
    TokenType_Semicolon,

    TokenType_Number,
    TokenType_Identifier,

    TokenType_EndOfStream,
};
struct token
{
    token_type Type;

    char *Text;
    umm Length;
};

struct tokenizer
{
    char *At;
};

#define CALC_TOKENIZER_H
#endif
