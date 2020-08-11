#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "calc.h"

int
main(int ArgCount, char **Args)
{
    if(ArgCount != 2)
    {
        printf("Usage: %s [expression]\n", Args[0]);
        return(1);
    }

    HMODULE CalcLibrary = LoadLibraryA("calc.dll");
    if(CalcLibrary)
    {
        calc_function *Calc = (calc_function *)GetProcAddress(CalcLibrary, "Calc");
        if(Calc)
        {
            char *Result = Calc(Args[1]);
            printf("%s\n", Result);
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

    return(0);
}
