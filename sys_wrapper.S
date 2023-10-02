#include <asm.h>
#include <segment.h>

ENTRY(write)
    pushl %ecx
    movl 8(%esp), %edx
    movl 12(%esp), %ecx
    movl 16(%esp), %ebx
    movl $4, %eax
    int $0x80
    popl %ecx
    ret

ENTRY(gettime)
    movl $10, %eax
    int $0x80
    ret
