#include "ppcasm.h"
#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)

#define FUNC_STUB(x)                   \
    void x()                           \
    {                                  \
        asm("li 0, " LINE_STRING ";"); \
        asm("li 0, " LINE_STRING ";"); \
        asm("li 0, " LINE_STRING ";"); \
        asm("li 0, " LINE_STRING ";"); \
        asm("li 0, " LINE_STRING ";"); \
    }

FUNC_STUB(CUtlVector_AddMultipleToTail_netadr)
