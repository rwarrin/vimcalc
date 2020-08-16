#include "calc.h"

#if NO_MALLOC
// TODO(rick): 16k is plenty... right?
static u8 Memory[Kilobytes(16)] = {0};
static struct calc_state CalcState = {0, 0, Memory, Kilobytes(16), 0};
#endif

#include "tokenizer.cpp"

static char PermanentMemory[256] = {0};

#define VARIABLE_SIGNIFICANT_NAME_LENGTH
struct variable_table_node
{
    struct calc_node *Value;
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

static void FreeNode(struct calc_node *Node);
static void
VariableTableInsert(char *Key, u32 Length, struct calc_node *VarNode)
{
    struct variable_table_node *Node = VariableTableFind(Key, Length);
    if(Node)
    {
        FreeNode(Node->Value);
        Node->Value = VarNode;
    }
    else
    {
        u32 KeyLength = VariableTableKeyLength(Length);
        u32 BucketIndex = VariableTableBucketIndex(Key, Length);

#ifndef NO_MALLOC
        Node = (struct variable_table_node *)malloc(sizeof(*Node));
#else
        Node = CalcState.VariableTableNodeFreeList;
        if(Node)
        {
            CalcState.VariableTableNodeFreeList = Node->Next;
            Node->Next = 0;
        }
        else
        {
            Node = PushStruct(&CalcState, struct variable_table_node);
        }
#endif
        memcpy(Node->Key, Key, KeyLength);
        Node->Key[KeyLength] = 0;
        Node->Value = VarNode;
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

#include "expression.cpp"

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

CALC_RESET(CalcReset)
{
    for(u32 BucketIndex = 0;
        BucketIndex < ArrayCount(VariableTable.Buckets);
        ++BucketIndex)
    {
        for(struct variable_table_node *Node = *(VariableTable.Buckets + BucketIndex); Node; )
        {
            struct variable_table_node *Temp = Node;
            Node = Node->Next;

            FreeNode(Temp->Value);

#ifndef NO_MALLOC
            free(Temp);
#else
            Temp->Next = CalcState.VariableTableNodeFreeList;
            CalcState.VariableTableNodeFreeList = Temp;
#endif
        }
    }
}

#ifdef __cplusplus
}
#endif
