#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "calc.h"

int
main(int ArgCount, char **Args)
{
    HMODULE CalcLibrary = LoadLibraryA("calc.dll");
    if(CalcLibrary)
    {
        calc_function *Calc = (calc_function *)GetProcAddress(CalcLibrary, "Calc");
        calc_reset_function *Reset = (calc_reset_function *)GetProcAddress(CalcLibrary, "CalcReset");
        if(Calc)
        {
            char ReadBuffer[256] = {0};
            while(fgets(ReadBuffer, ArrayCount(ReadBuffer), stdin) != 0)
            {
                printf("%s\n", Calc(ReadBuffer));
            }

            if(Reset)
            {
                Reset();
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

    return(0);
}
