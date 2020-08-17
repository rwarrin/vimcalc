#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "calc.h"

#define test_func(Type, Name) \
    Type *Name = (Type *)GetProcAddress(CalcLibrary, #Name); \
    if(Name) \
    { \
        while(fgets(ReadBuffer, ArrayCount(ReadBuffer), stdin) != 0) \
        { \
            printf("%s\n", Name(ReadBuffer)); \
        } \
    }

int
main(int ArgCount, char **Args)
{
    HMODULE CalcLibrary = LoadLibraryA("calc.dll");
    if(CalcLibrary)
    {
        char ReadBuffer[256] = {0};
        test_func(calc_function, Calc)
        //test_func(calc_reset_function, CalcReset)
        //test_func(calc_to_binary_function, ToBinary)
        //test_func(calc_to_hex, ToHex)
        //test_func(calc_from_hex, FromHex)
        else
        {
            fprintf(stderr, "Failed to get proc address\n");
        }
    }
    else
    {
        fprintf(stderr, "Failed to load library\n");
    }

    return(0);
}
