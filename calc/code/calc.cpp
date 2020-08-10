#include "calc.h"

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

inline b32
IsWhitespace(char Character)
{
    b32 Result = ((Character == ' ') ||
                  (Character == '\t') ||
                  (Character == '\r') ||
                  (Character == '\n'));
    return(Result);
}

inline b32
IsDigit(char Character)
{
    b32 Result = ((Character >= '0') && (Character <= '9'));
    return(Result);
}

inline void
ConsumeWhitespace(struct tokenizer *Tokenizer)
{
    while(IsWhitespace(Tokenizer->At[0]))
    {
        ++Tokenizer->At;
    }
}

static struct token
GetToken(struct tokenizer *Tokenizer)
{
    ConsumeWhitespace(Tokenizer);

    struct token Token = {};
    Token.Type = TokenType_Unknown;
    Token.Text = Tokenizer->At;
    Token.Length = 1;
    
    char Character = Tokenizer->At[0];
    ++Tokenizer->At;

    switch(Character)
    {
        case '\0': { Token.Type = TokenType_EndOfStream; } break;
        case '(': { Token.Type = TokenType_OpenParen; } break;
        case ')': { Token.Type = TokenType_CloseParen; } break;
        case '*': { Token.Type = TokenType_Asterisk; } break;
        case '-': { Token.Type = TokenType_Minus; } break;
        case '+': { Token.Type = TokenType_Plus; } break;
        case '/': { Token.Type = TokenType_ForwardSlash; } break;
        case '%': { Token.Type = TokenType_Percent; } break;
        case '|': { Token.Type = TokenType_Pipe; } break;
        case '&': { Token.Type = TokenType_Ampersand; } break;
        case '^': { Token.Type = TokenType_Carat; } break;
        case '=': { Token.Type = TokenType_Equals; } break;
        case ';': { Token.Type = TokenType_Semicolon; } break;
        
        default:
        {
            if(IsDigit(Character))
            {
                // TODO(rick): Improve this to parse floating point constants
                // like 1.0f
                Token.Type = TokenType_Number;
                while(IsDigit(*Tokenizer->At))
                {
                    ++Tokenizer->At;
                    Token.Length = Tokenizer->At - Token.Text;
                }
            }
            else
            {
                InvalidCodePath;
            }
        } break;
    }

    return(Token);
}

static struct token
PeekToken(struct tokenizer *Tokenizer)
{
    struct tokenizer Tokenizer2 = *Tokenizer;
    struct token Result = GetToken(&Tokenizer2);
    return(Result);
}

enum calc_node_type
{
    CalcNode_UnaryMinus = 0,
    CalcNode_Add,
    CalcNode_Subtract,
    CalcNode_Multiply,
    CalcNode_Divide,
    CalcNode_Modulus,
    CalcNode_BinaryOr,
    CalcNode_BinaryAnd,
    CalcNode_BinaryXor,
    CalcNode_Constant,

    CalcNode_Variable,
};
struct calc_node
{
    calc_node_type Type;
    r64 Value;
    struct calc_node *Left;
    struct calc_node *Right;
};

static struct calc_node *
AddNode(calc_node_type Type, struct calc_node *Left = 0, struct calc_node *Right = 0)
{
    calc_node *Node = (calc_node *)malloc(sizeof(*Node));
    Node->Type = Type;
    Node->Value = 0.0;
    Node->Left = Left;
    Node->Right = Right;
    return(Node);
}

static void
FreeNode(struct calc_node *Node)
{
    if(Node)
    {
        FreeNode(Node->Left);
        FreeNode(Node->Right);
        free(Node);
    }
}

static calc_node *
ParseNumber(struct tokenizer *Tokenizer)
{
    struct calc_node *Result = 0;

    struct token Token = GetToken(Tokenizer);
    Result->Value = atof(Token.Text);

    return(Result);
}

static calc_node *
ParseConstant(struct tokenizer *Tokenizer)
{
    calc_node *Result = 0;

    struct token Token = PeekToken(Tokenizer);
    if(Token.Type == TokenType_Minus)
    {
        Token = GetToken(Tokenizer);
        Result = AddNode(CalcNode_UnaryMinus);
        Result->Left = ParseNumber(Tokenizer);
    }
    else
    {
        Result = ParseNumber(Tokenizer);
    }

    return(Result);
}

static calc_node *
ParseMultiplyExpression(struct tokenizer *Tokenizer)
{
    struct calc_node *Result = 0;

    struct token Token = GetToken(Tokenizer);
    if((Token.Type == TokenType_Minus) ||
       (Token.Type == TokenType_Number))
    {
        Result = ParseConstant(Tokenizer);
        
        struct token Token = PeekToken(Tokenizer);
        if(Token.Type == TokenType_ForwardSlash)
        {
            GetToken(Tokenizer);
            Result = AddNode(CalcNode_Divide, Result, ParseNumber(Tokenizer));
        }
        else if(Token.Type == TokenType_Asterisk)
        {
            GetToken(Tokenizer);
            Result = AddNode(CalcNode_Multiply, Result, ParseNumber(Tokenizer));
        }
    }

    return(Result);
}

static calc_node *
ParseAddExpression(struct tokenizer *Tokenizer)
{
    struct calc_node *Result = 0;

    struct token Token = GetToken(Tokenizer);
    if((Token.Type == TokenType_Minus) ||
       (Token.Type == TokenType_Number))
    {
        Result = ParseMultiplyExpression(Tokenizer);

        struct token Token = PeekToken(Tokenizer);
        if(Token.Type == TokenType_Plus)
        {
            GetToken(Tokenizer);
            Result = AddNode(CalcNode_Add, Result, ParseAddExpression(Tokenizer));
        }
        else if(Token.Type == TokenType_Minus)
        {
            GetToken(Tokenizer);
            Result = AddNode(CalcNode_Subtract, Result, ParseAddExpression(Tokenizer));
        }
    }

    return(Result);
}

static r32
ParseExpression(struct tokenizer *Tokenizer)
{
    // TODO(rick): Actually parse an expression
    r32 Result = 0;
    struct token Token = {};
    for(;;)
    {
        Token = GetToken(Tokenizer);
        printf("%d (%d) %.*s\n", Token.Type, (u32)Token.Length,
               (u32)Token.Length, Token.Text);

        if(Token.Type == TokenType_EndOfStream)
        {
            break;
        }
    }

    Assert(Token.Text[0] == 0);
    return(Result);
}

int
main(int ArgCount, char **Args)
{
    printf("Hello World\n");
    struct tokenizer Tokenizer = {};

    Tokenizer.At = Args[1];
    ParseExpression(&Tokenizer);

    return(0);
}
