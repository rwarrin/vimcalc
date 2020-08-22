#include "calc.h"
#include <windows.h>

#include "tokenizer.cpp"


static char PermanentMemory[256] = {0};
static struct variable_table *VariableTable;
static struct calc_state *CalcState;
static u8 *GlobalMemory = 0;

static void
InitCalcState(struct calc_state *State, u8 *Memory)
{
    u32 HeadersSize = (sizeof(struct calc_state) + sizeof(struct variable_table));
    State->Initialized = true;
    State->CalcNodeFreeList = 0;
    State->VariableTableNodeFreeList = 0;
    State->Size = STORAGE_MEMORY_SIZE - HeadersSize;
    State->Used = 0;
    State->Memory = Memory + HeadersSize;
}

static void
LoadCalcState()
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
            InitCalcState(CalcState, GlobalMemory);
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
    // TODO(rick): Better hash function!
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

CALC_MEM_RESET(CalcReset)
{
    LoadCalcState();
    memset(GlobalMemory, 0, STORAGE_MEMORY_SIZE);
    InitCalcState(CalcState, GlobalMemory);
}

CALC_INFO(Info)
{
    LoadCalcState();
    snprintf(PermanentMemory, ArrayCount(PermanentMemory),
             "Init: %d, Node FL: %p, VT FL: %p, M: %p, Size: %zd, Used: %zd",
             CalcState->Initialized, CalcState->CalcNodeFreeList,
             CalcState->VariableTableNodeFreeList, CalcState->Memory,
             CalcState->Size, CalcState->Used);
    return(PermanentMemory);
}

#ifdef __cplusplus
}
#endif
