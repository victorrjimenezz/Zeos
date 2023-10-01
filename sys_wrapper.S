#include <asm.h>
#inclue <segment.h>

ENTRY(gettime)
    movl $10, %eax; \
    int $0x80; \
    ret