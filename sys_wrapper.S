#include <asm.h>
#include <segment.h>

ENTRY(write)
    movl 4(%esp), %edx
    movl 8(%esp), %ecx
    movl 12(%esp), %ebx
    movl $4, %eax
    int $0x80
    ret

ENTRY(gettime)
    movl $10, %eax
    int $0x80
    ret
