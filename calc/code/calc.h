#ifndef CALC_H

#include <stdio.h>
#include <stdlib.h>

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

#define ArrayCount(Array) (sizeof((Array)) / sizeof((Array)[0]))
#define Assert(Condition) if(!(Condition)) { *(int *)0 = 0; }

#define InvalidCodePath Assert(!"InvalidCodePath")

#define CALC(name) char * name(char *Expression)
typedef CALC(calc_function);

#define CALC_H
#endif
