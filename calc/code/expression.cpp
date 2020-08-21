
static struct calc_node *
AddNode(calc_node_type Type, struct calc_node *Left = 0, struct calc_node *Right = 0)
{
    calc_node *Node = CalcState->CalcNodeFreeList;
    if(Node)
    {
        CalcState->CalcNodeFreeList = Node->Left;
        Node->Left = 0;
    }
    else
    {
        Node = PushStruct(CalcState, struct calc_node);
        if(!Node)
        {
            longjmp(ErrorJump, EXCEPTYPE_OUT_OF_MEMORY);
        }
    }
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
        Node->Left = CalcState->CalcNodeFreeList;
        CalcState->CalcNodeFreeList = Node;
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
        Result->Left = ParseConstant(Tokenizer);
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

static calc_node *ParseParenthesisExpression(struct tokenizer *Tokenizer);
static calc_node *ParseBitwiseExpression(struct tokenizer *Tokenizer);
static calc_node *
ParseMultiplyExpression(struct tokenizer *Tokenizer)
{
    struct calc_node *Result = 0;

    struct token Token = PeekToken(Tokenizer);
    if(Token.Type == TokenType_OpenParen)
    {
        Result = ParseParenthesisExpression(Tokenizer);
    }
    else if((Token.Type == TokenType_Minus) ||
       (Token.Type == TokenType_Number) ||
       (Token.Type == TokenType_Identifier) ||
       (Token.Type == TokenType_Tilde))
    {
        Result = ParseConstant(Tokenizer);
        
        struct token Token = PeekToken(Tokenizer);
        if(Token.Type == TokenType_ForwardSlash)
        {
            GetToken(Tokenizer);
            Result = AddNode(CalcNode_Divide, Result, ParseMultiplyExpression(Tokenizer));
        }
        else if(Token.Type == TokenType_Asterisk)
        {
            GetToken(Tokenizer);
            Result = AddNode(CalcNode_Multiply, Result, ParseMultiplyExpression(Tokenizer));
        }
        else if(Token.Type == TokenType_Percent)
        {
            GetToken(Tokenizer);
            Result = AddNode(CalcNode_Modulus, Result, ParseMultiplyExpression(Tokenizer));
        }
    }

    return(Result);
}

static calc_node *
ParseAddExpression(struct tokenizer *Tokenizer)
{
    struct calc_node *Result = 0;

    struct token Token = PeekToken(Tokenizer);
    if(Token.Type == TokenType_OpenParen)
    {
        Result = ParseParenthesisExpression(Tokenizer);
    }
    else if((Token.Type == TokenType_Minus) ||
       (Token.Type == TokenType_Number) ||
       (Token.Type == TokenType_Identifier) ||
       (Token.Type == TokenType_Tilde))
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

static struct calc_node *
ParseVariableAssignment(struct tokenizer *Tokenizer)
{
    struct calc_node *Result = 0;
    struct token Token = PeekToken(Tokenizer);
    if(Token.Type == TokenType_Identifier)
    {
        struct token NameToken = GetToken(Tokenizer);
        Token = PeekToken(Tokenizer);
        if(Token.Type == TokenType_Equals)
        {
            GetToken(Tokenizer);
            Token = PeekToken(Tokenizer);
#if 0
            if(Token.Type == TokenType_Number)
            {
                r64 Value = ParseNumberValue(Tokenizer);
                VariableTableInsert(NameToken.Text, (u32)NameToken.Length, Value);
            }
#else
            Result = ParseBitwiseExpression(Tokenizer);
            Token = PeekToken(Tokenizer);
            if(Token.Type == TokenType_Semicolon)
            {
                GetToken(Tokenizer);
            }

            VariableTableInsert(NameToken.Text, (u32)NameToken.Length, Result);
#endif
        }
    }

    return(Result);
}

static calc_node *
ParseParenthesisExpression(struct tokenizer *Tokenizer)
{
    struct calc_node *Result = 0;
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

    return(Result);
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
        Result = ParseParenthesisExpression(Tokenizer);
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
                    Result = ExecuteCalcNode(Node->Variable->Value);
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
    LoadCalcState();

    struct calc_node *Node = ParseBitwiseExpression(Tokenizer);
    r64 Result = ExecuteCalcNode(Node);
    printf("Result: %f\n", Result);
    FreeNode(Node);

    return(Result);
}

