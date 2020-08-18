
#include "calc.h"

static u8 Memory[Kilobytes(8)] = {0};

#ifdef __cplusplus
extern "C" {
#endif

CALC_MEM_LOAD(LoadCalcMemory)
{
    u8 *Result = Memory;
    return(Result);
}

#ifdef __cplusplus
}
#endif
