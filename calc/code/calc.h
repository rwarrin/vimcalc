#ifndef CALC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <signal.h>
#include <stdint.h>

#include <math.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef int32_t b32;
typedef float r32;
typedef double r64;
typedef size_t umm;

#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define MIN(A, B) ((A) < (B) ? (A) : (B))

#define ArrayCount(Array) (sizeof((Array)) / sizeof((Array)[0]))
#define Assert(Condition) if(!(Condition)) { *(int *)0 = 0; }

#define InvalidCodePath Assert(!"InvalidCodePath")

#define CALC_MEM_LOAD(name) u8 *name(void)
typedef CALC_MEM_LOAD(calc_mem_load_function);

#define CALC(name) char * name(char *Expression)
typedef CALC(calc_function);

#define CALC_TO_BINARY(name) char *name(char *Expression)
typedef CALC_TO_BINARY(calc_to_binary_function);

#define CALC_TO_HEX(name) char *name(char *Expression)
typedef CALC_TO_HEX(calc_to_hex);

#define CALC_FROM_HEX(name) char *name(char *String)
typedef CALC_FROM_HEX(calc_from_hex);

#define CALC_MEM_RESET(name) void name(void)
typedef CALC_MEM_RESET(calc_mem_reset_function);

#define CALC_INFO(name) char *name(void)
typedef CALC_INFO(calc_info_function);

enum excep_type
{
    EXCEPTYPE_OUT_OF_MEMORY = 1,
};

struct calc_node;
struct variable_table_node;
struct calc_state
{
    b32 Initialized;
    struct calc_node *CalcNodeFreeList;
    struct variable_table_node *VariableTableNodeFreeList;
    u8 *Memory;
    umm Size;
    umm Used;
};

#define Kilobytes(Value) (Value*1024LL)
#define PushStruct(CalcState, Type) (Type *)PushSize_(CalcState, sizeof(Type))

void *
PushSize_(struct calc_state *CalcState, umm Size, u32 Alignment = 8)
{
    void *Result = 0;

    umm AlignmentOffset = 0;
    umm ResultPointer = (umm)CalcState->Memory + CalcState->Used;
    umm AlignmentMask = Alignment - 1;
    if(ResultPointer & AlignmentMask)
    {
        AlignmentOffset = Alignment - (ResultPointer & AlignmentMask);
    }

    umm EffectiveSize = Size + AlignmentOffset;

    if(CalcState->Used + EffectiveSize < CalcState->Size)
    {
        Result = CalcState->Memory + CalcState->Used + AlignmentOffset;
        CalcState->Used += EffectiveSize;
    }

    return(Result);
}

#define VARIABLE_SIGNIFICANT_NAME_LENGTH 16
struct variable_table_node
{
    struct calc_node *Value;
    char Key[VARIABLE_SIGNIFICANT_NAME_LENGTH];
    struct variable_table_node *Next;
};

struct variable_table
{
    struct variable_table_node *Buckets[32];
};

struct memory_map
{
    struct calc_state CalcState;
    struct variable_table VariableTable;
    u8 *Memory;
};

jmp_buf ErrorJump;

#define STORAGE_MEMORY_SIZE Kilobytes(32)

#include "tokenizer.h"
#include "expression.h"

#define CALC_H
#endif
