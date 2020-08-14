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

inline b32
IsAlpha(char Character)
{
    b32 Result = (((Character >= 'a') && (Character <= 'z')) ||
                  ((Character >= 'A') && (Character <= 'Z')));
    return(Result);
}

inline b32
IsPunctuation(char Character)
{
    b32 Result = (Character == '_');
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
        case '~': { Token.Type = TokenType_Tilde; } break;
        case '<': { Token.Type = TokenType_LessThan; } break;
        case '>': { Token.Type = TokenType_GreaterThan; } break;
        
        default:
        {
            if(IsDigit(Character))
            {
                // TODO(rick): Improve this to parse floating point constants
                // like 1.0f
                Token.Type = TokenType_Number;
                while(IsDigit(Tokenizer->At[0]) ||
                      (Tokenizer->At[0] == '.') ||
                      (Tokenizer->At[0] == 'f'))
                {
                    ++Tokenizer->At;
                }
                Token.Length = Tokenizer->At - Token.Text;
            }
            else if(IsAlpha(Character) ||
                    IsPunctuation(Character))
            {
                Token.Type = TokenType_Identifier;
                while(IsAlpha(Tokenizer->At[0]) ||
                      IsDigit(Tokenizer->At[0]) ||
                      IsPunctuation(Tokenizer->At[0]))
                {
                    ++Tokenizer->At;
                }
                Token.Length = Tokenizer->At - Token.Text;
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

#define VARIABLE_SIGNIFICANT_NAME_LENGTH
struct variable_table_node
{
    r64 Value;
    char Key[16];
    struct variable_table_node *Next;
};

struct variable_table
{
    struct variable_table_node *Buckets[32];
};

static struct variable_table VariableTable;

inline u32
VariableTableKeyLength(u32 Length)
{
    u32 Result = MIN(Length, VARIABLE_SIGNIFICANT_NAME_LENGTH - 1);
    return(Result);
}

inline u32
HashKey(char *Key, u32 Length)
{
    u32 Result = 5381;
    
    while(Length--)
    {
        Result = ((Result << 5) + Result) + *Key;
    }

    return(Result);
}

inline u32
VariableTableBucketIndex(char *Key, u32 Length)
{
    u32 Result = 0;
    u32 KeyLength = VariableTableKeyLength(Length);
    u32 Hash = HashKey(Key, KeyLength);
    Result = (Hash % ArrayCount(VariableTable.Buckets));
    return(Result);
}

static struct variable_table_node * 
VariableTableFind(char *Key, u32 Length)
{
    struct variable_table_node *Result = 0;

    u32 KeyLength = VariableTableKeyLength(Length);
    u32 BucketIndex = VariableTableBucketIndex(Key, Length);
    for(struct variable_table_node *Node = VariableTable.Buckets[BucketIndex];
        Node;
        Node = Node->Next)
    {
        if(strncmp(Key, Node->Key, KeyLength) == 0)
        {
            Result = Node;
            break;
        }
    }

    return(Result);
}

static void
VariableTableInsert(char *Key, u32 Length, r64 Value)
{
    struct variable_table_node *Node = VariableTableFind(Key, Length);
    if(Node)
    {
        Node->Value = Value;
    }
    else
    {
        u32 KeyLength = VariableTableKeyLength(Length);
        u32 BucketIndex = VariableTableBucketIndex(Key, Length);

        Node = (struct variable_table_node *)malloc(sizeof(*Node));
        memcpy(Node->Key, Key, KeyLength);
        Node->Key[KeyLength] = 0;
        Node->Value = Value;
        Node->Next = VariableTable.Buckets[BucketIndex];
        VariableTable.Buckets[BucketIndex] = Node;
    }
}

static struct variable_table_node *
VariableTableGet(char *Key, u32 Length)
{
    struct variable_table_node *Result = 0;

    struct variable_table_node *Node = VariableTableFind(Key, Length);
    if(Node)
    {
        Result = Node;
    }

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

    CalcNode_BitwiseAnd,
    CalcNode_BitwiseOr,
    CalcNode_BitwiseXor,
    CalcNode_BitwiseNot,
    CalcNode_BitwiseLeftShift,
    CalcNode_BitwiseRightShift,

    CalcNode_Constant,
    CalcNode_Variable,
};
struct calc_node
{
    union
    {
        r64 R64Value;
        struct variable_table_node *Variable;
    };
    struct calc_node *Left;
    struct calc_node *Right;
    calc_node_type Type;
};

static struct calc_node *
AddNode(calc_node_type Type, struct calc_node *Left = 0, struct calc_node *Right = 0)
{
    calc_node *Node = (calc_node *)malloc(sizeof(*Node));
    Node->Type = Type;
    Node->R64Value = 0.0;
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

static r64
ParseNumberValue(struct tokenizer *Tokenizer)
{
    r64 Result = 0;

    struct token Token = GetToken(Tokenizer);
    Result = atof(Token.Text);

    return(Result);
}

static calc_node *
ParseNumber(struct tokenizer *Tokenizer)
{
    struct calc_node *Result = AddNode(CalcNode_Constant);

    struct token Token = GetToken(Tokenizer);
    Result->R64Value = atof(Token.Text);

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
    else if(Token.Type == TokenType_Identifier)
    {
        Token = GetToken(Tokenizer);
        Result = AddNode(CalcNode_Variable);
        Result->Variable = VariableTableGet(Token.Text, (u32)Token.Length);
    }
    else
    {
        Result = ParseNumber(Tokenizer);
    }

    return(Result);
}

static calc_node *ParseAddExpression(struct tokenizer *Tokenizer);
static calc_node *ParseBitwiseExpression(struct tokenizer *Tokenizer);
static calc_node *
ParseMultiplyExpression(struct tokenizer *Tokenizer)
{
    struct calc_node *Result = 0;

    struct token Token = PeekToken(Tokenizer);
    if((Token.Type == TokenType_Minus) ||
       (Token.Type == TokenType_Number) ||
       (Token.Type == TokenType_Identifier) ||
       (Token.Type == TokenType_Tilde))
    {
        Result = ParseConstant(Tokenizer);
        
        struct token Token = PeekToken(Tokenizer);
        if(Token.Type == TokenType_ForwardSlash)
        {
            GetToken(Tokenizer);
            Result = AddNode(CalcNode_Divide, Result, ParseAddExpression(Tokenizer));
        }
        else if(Token.Type == TokenType_Asterisk)
        {
            GetToken(Tokenizer);
            Result = AddNode(CalcNode_Multiply, Result, ParseAddExpression(Tokenizer));
        }
        else if(Token.Type == TokenType_Percent)
        {
            GetToken(Tokenizer);
            Result = AddNode(CalcNode_Modulus, Result, ParseAddExpression(Tokenizer));
        }
    }

    return(Result);
}

static calc_node *
ParseAddExpression(struct tokenizer *Tokenizer)
{
    struct calc_node *Result = 0;

    struct token Token = PeekToken(Tokenizer);
    if((Token.Type == TokenType_Minus) ||
       (Token.Type == TokenType_Number) ||
       (Token.Type == TokenType_Identifier) ||
       (Token.Type == TokenType_Tilde))
    {
        Result = ParseMultiplyExpression(Tokenizer);

        struct token Token = PeekToken(Tokenizer);
        if(Token.Type == TokenType_Plus)
        {
            GetToken(Tokenizer);
            Result = AddNode(CalcNode_Add, Result, ParseBitwiseExpression(Tokenizer));
        }
        else if(Token.Type == TokenType_Minus)
        {
            GetToken(Tokenizer);
            Result = AddNode(CalcNode_Subtract, Result, ParseBitwiseExpression(Tokenizer));
        }
    }

    return(Result);
}

static void
ParseVariableAssignment(struct tokenizer *Tokenizer)
{
    struct token Token = PeekToken(Tokenizer);
    if(Token.Type == TokenType_Identifier)
    {
        struct token NameToken = GetToken(Tokenizer);
        Token = PeekToken(Tokenizer);
        if(Token.Type == TokenType_Equals)
        {
            GetToken(Tokenizer);
            Token = PeekToken(Tokenizer);
            if(Token.Type == TokenType_Number)
            {
                r64 Value = ParseNumberValue(Tokenizer);
                VariableTableInsert(NameToken.Text, (u32)NameToken.Length, Value);
            }
        }
    }
}

static calc_node *
ParseBitwiseExpression(struct tokenizer *Tokenizer)
{
    struct calc_node *Result = 0;

    struct token Token = PeekToken(Tokenizer);
    if(Token.Type == TokenType_Identifier)
    {
        struct tokenizer TokenizerCopy = *Tokenizer;
        GetToken(&TokenizerCopy);
        struct token NextToken = PeekToken(&TokenizerCopy);
        if(NextToken.Type == TokenType_Equals)
        {
            ParseVariableAssignment(Tokenizer);
            return Result = ParseBitwiseExpression(Tokenizer);
        }
    }

    if(Token.Type == TokenType_OpenParen)
    {
        // TODO(rick): Clean this up
        GetToken(Tokenizer);
        struct calc_node *SubExpression = ParseBitwiseExpression(Tokenizer);

        // NOTE(rick): We've pulled out the entire sub expression at this point
        // and so we're going to insert a number to restart the parser for the
        // larger expression.
        Assert(PeekToken(Tokenizer).Type == TokenType_CloseParen);
        *Tokenizer->At = '1';
        Result = ParseBitwiseExpression(Tokenizer);

        // NOTE(rick): At this point we've found the reset of the expression and
        // now we need to insert the subexpression into the tree. Find the last
        // node on the left, this will be the sentinel '1' we injected into the
        // stream.
        struct calc_node *Searcher = Result;
        while(Searcher->Left)
        {
            Searcher = Searcher->Left;
        }
        Assert(Searcher->Type == CalcNode_Constant);
        // NOTE(rick): Replace the sentinel with the sub expression.
        *Searcher = *SubExpression;
    }
    else if((Token.Type == TokenType_Minus) ||
       (Token.Type == TokenType_Number) ||
       (Token.Type == TokenType_Identifier) ||
       (Token.Type == TokenType_Tilde))
    {
        if(Token.Type == TokenType_Tilde)
        {
            GetToken(Tokenizer);
            Result = AddNode(CalcNode_BitwiseNot, ParseBitwiseExpression(Tokenizer), 0);
        }
        else
        {
            Result = ParseAddExpression(Tokenizer);

            struct token Token = PeekToken(Tokenizer);
            if(Token.Type == TokenType_Ampersand)
            {
                GetToken(Tokenizer);
                Result = AddNode(CalcNode_BitwiseAnd, Result, ParseBitwiseExpression(Tokenizer));
            }
            else if(Token.Type == TokenType_Pipe)
            {
                GetToken(Tokenizer);
                Result = AddNode(CalcNode_BitwiseOr, Result, ParseBitwiseExpression(Tokenizer));
            }
            else if(Token.Type == TokenType_Carat)
            {
                GetToken(Tokenizer);
                Result = AddNode(CalcNode_BitwiseXor, Result, ParseBitwiseExpression(Tokenizer));
            }
            else if(Token.Type == TokenType_LessThan)
            {
                GetToken(Tokenizer);
                Token = PeekToken(Tokenizer);
                {
                    if(Token.Type == TokenType_LessThan)
                    {
                        GetToken(Tokenizer);
                        Result = AddNode(CalcNode_BitwiseLeftShift, Result, ParseBitwiseExpression(Tokenizer));
                    }
                }
            }
            else if(Token.Type == TokenType_GreaterThan)
            {
                GetToken(Tokenizer);
                Token = PeekToken(Tokenizer);
                {
                    if(Token.Type == TokenType_GreaterThan)
                    {
                        GetToken(Tokenizer);
                        Result = AddNode(CalcNode_BitwiseRightShift, Result, ParseBitwiseExpression(Tokenizer));
                    }
                }
            }
        }
    }

    return(Result);
}

static r64
ExecuteCalcNode(struct calc_node *Node)
{
    r64 Result = 0.0f;

    if(Node)
    {
        switch(Node->Type)
        {
            case CalcNode_UnaryMinus:
            {
                Result = -(ExecuteCalcNode(Node->Left));
            } break;

            case CalcNode_Add:
            {
                Result = ExecuteCalcNode(Node->Left) + ExecuteCalcNode(Node->Right);
            } break;

            case CalcNode_Subtract:
            {
                Result = ExecuteCalcNode(Node->Left) - ExecuteCalcNode(Node->Right);
            } break;

            case CalcNode_Multiply:
            {
                Result = ExecuteCalcNode(Node->Left) * ExecuteCalcNode(Node->Right);
            } break;

            case CalcNode_Divide:
            {
                Result = ExecuteCalcNode(Node->Left) / ExecuteCalcNode(Node->Right);
            } break;

            case CalcNode_Modulus:
            {
                Result = fmod(ExecuteCalcNode(Node->Left), ExecuteCalcNode(Node->Right));
            } break;

            case CalcNode_BitwiseAnd:
            {
                Result = (r64)((u64)ExecuteCalcNode(Node->Left) & (u64)ExecuteCalcNode(Node->Right));
            } break;

            case CalcNode_BitwiseOr:
            {
                Result = (r64)((u64)ExecuteCalcNode(Node->Left) | (u64)ExecuteCalcNode(Node->Right));
            } break;

            case CalcNode_BitwiseXor:
            {
                Result = (r64)((u64)ExecuteCalcNode(Node->Left) ^ (u64)ExecuteCalcNode(Node->Right));
            } break;

            case CalcNode_BitwiseNot:
            {
                r64 Value = ExecuteCalcNode(Node->Left);
                s64 Bits = ~(s64)(Value);
                Result = (r64)(Bits);
            } break;

            case CalcNode_BitwiseLeftShift:
            {
                Result = (r64)((u64)ExecuteCalcNode(Node->Left) << (u64)ExecuteCalcNode(Node->Right));
            } break;

            case CalcNode_BitwiseRightShift:
            {
                Result = (r64)((u64)ExecuteCalcNode(Node->Left) >> (u64)ExecuteCalcNode(Node->Right));
            } break;

            case CalcNode_Variable:
            {
                if(Node->Variable)
                {
                    Result = Node->Variable->Value;
                }
            } break;

            case CalcNode_Constant:
            {
                Result = Node->R64Value;
            } break;
        }
    }

    return(Result);
}

static r64
ParseExpression(struct tokenizer *Tokenizer)
{
    struct calc_node *Node = ParseBitwiseExpression(Tokenizer);
    r64 Result = ExecuteCalcNode(Node);
    printf("Result: %f\n", Result);

    FreeNode(Node);

    return(Result);
}

static char PermanentMemory[256] = {0};

#ifdef __cplusplus
extern "C" {
#endif

CALC(Calc)
{
    struct tokenizer Tokenizer = {};
    Tokenizer.At = Expression;
    r64 ComputedResult = ParseExpression(&Tokenizer);

    snprintf(PermanentMemory, ArrayCount(PermanentMemory), "%f", ComputedResult);
    return(PermanentMemory);
}

#ifdef __cplusplus
}
#endif
