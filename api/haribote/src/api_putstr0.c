#include "haribote.h"

void api_putstr0(const char *s)
{
    __asm__ volatile(
        "int   $0x40\n"

        :
        : "d" (API_PUTSTR0), "b" (s)
    );
}
