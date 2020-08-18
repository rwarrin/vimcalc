#include "calc.h"

#if NO_MALLOC
// TODO(rick): 8k is plenty... right?
//static u8 Memory[Kilobytes(8)] = {0};
//static struct calc_state CalcState = {0, 0, Memory, Kilobytes(8), 0};
#endif

#include <windows.h>


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

static struct variable_table *VariableTable;

static struct calc_state *CalcState;
struct memory_map
{
    struct calc_state CalcState;
    struct variable_table VariableTable;
    u8 *Memory;
};
static u8 *GlobalMemory = 0;
static void LoadCalcState()
{
    HMODULE CalcMemLibrary = LoadLibraryA("calc_memory.dll");
    calc_mem_load_function *LoadCalcMemory = (calc_mem_load_function *)GetProcAddress(CalcMemLibrary, "LoadCalcMemory");
    if(LoadCalcMemory)
    {
        GlobalMemory = LoadCalcMemory();
        b32 Initialized = *(b32 *)GlobalMemory;

        struct memory_map *Map = (struct memory_map *)GlobalMemory;
        CalcState = &Map->CalcState;
        VariableTable = &Map->VariableTable;

        if(Initialized)
        {
            FreeLibrary(CalcMemLibrary);
        }
        else
        {
            CalcState->Initialized = true;
            CalcState->CalcNodeFreeList = 0;
            CalcState->VariableTableNodeFreeList = 0;
            CalcState->Size = Kilobytes(8) - (sizeof(struct calc_state) + sizeof(struct variable_table));
            CalcState->Used = 0;
            CalcState->Memory = GlobalMemory + (sizeof(struct calc_state) + sizeof(struct variable_table));
        }
    }
}

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
    Result = (Hash % ArrayCount(VariableTable->Buckets));
    return(Result);
}

static struct variable_table_node * 
VariableTableFind(char *Key, u32 Length)
{
    struct variable_table_node *Result = 0;

    u32 KeyLength = VariableTableKeyLength(Length);
    u32 BucketIndex = VariableTableBucketIndex(Key, Length);
    for(struct variable_table_node *Node = VariableTable->Buckets[BucketIndex];
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
        Node = CalcState->VariableTableNodeFreeList;
        if(Node)
        {
            CalcState->VariableTableNodeFreeList = Node->Next;
            Node->Next = 0;
        }
        else
        {
            Node = PushStruct(CalcState, struct variable_table_node);
        }
#endif
        memcpy(Node->Key, Key, KeyLength);
        Node->Key[KeyLength] = 0;
        Node->Value = VarNode;
        Node->Next = VariableTable->Buckets[BucketIndex];
        VariableTable->Buckets[BucketIndex] = Node;
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
        BucketIndex < ArrayCount(VariableTable->Buckets);
        ++BucketIndex)
    {
        for(struct variable_table_node *Node = *(VariableTable->Buckets + BucketIndex); Node; )
        {
            struct variable_table_node *Temp = Node;
            Node = Node->Next;

            FreeNode(Temp->Value);

#ifndef NO_MALLOC
            free(Temp);
#else
            Temp->Next = CalcState->VariableTableNodeFreeList;
            CalcState->VariableTableNodeFreeList = Temp;
#endif
        }
    }
}

CALC_TO_BINARY(ToBinary)
{
    struct tokenizer Tokenizer = {};
    Tokenizer.At = Expression;
    r64 ComputedResult = ParseExpression(&Tokenizer);
    s64 Number = (s64)ComputedResult;

    char *At = PermanentMemory;
    for(s32 Index = 63; Index >= 0; --Index)
    {
        *At++ = (Number & ((u64)1  << Index)) ? '1' : '0';
        if(Index % 8 == 0)
        {
            *At++ = ' ';
        }

    }
    *At = 0;

    return(PermanentMemory);
}

CALC_TO_HEX(ToHex)
{
    struct tokenizer Tokenizer = {};
    Tokenizer.At = Expression;
    r64 ComputedResult = ParseExpression(&Tokenizer);
    s64 Number = (s64)ComputedResult;

    snprintf(PermanentMemory, sizeof(PermanentMemory), "0x%llX", Number);
    return(PermanentMemory);
}

CALC_FROM_HEX(FromHex)
{
    s64 Number = (s64)atof(String);
    snprintf(PermanentMemory, sizeof(PermanentMemory), "%lld", Number);
    return(PermanentMemory);
}

#ifdef __cplusplus
}
#endif
