
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

