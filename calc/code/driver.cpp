#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "calc.h"

#define get_func(Type, Name) \
    Type *Name = (Type *)GetProcAddress(CalcLibrary, #Name);
#define test_func(Type, Name) \
    get_func(Type, Name) \
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
        //test_func(calc_reset_function, CalcReset)
        //test_func(calc_to_binary_function, ToBinary)
        //test_func(calc_to_hex, ToHex)
        //test_func(calc_from_hex, FromHex)
        get_func(calc_info_function, Info);
        get_func(calc_function, Calc)
        char ReadBuffer[256] = {0};
        if(Calc && Info)
        {
            while(fgets(ReadBuffer, ArrayCount(ReadBuffer), stdin) != 0)
            {
                printf("%s\n", Calc(ReadBuffer));
                printf("%s\n", Info());
            }
        }
        else
        {
            fprintf(stderr, "Failed to get proc address\n");
        }
    }
    else
    {
        fprintf(stderr, "Failed to load library\n");
    }

    FreeLibrary(CalcLibrary);
    return(0);
}
