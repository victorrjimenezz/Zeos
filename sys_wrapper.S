#include <asm.h>
#include <segment.h>

ENTRY(write)
    pushl %ebp
    pushl %ebx
    pushl r_write
    movl %esp, %ebp
    movl 16(%esp), %edx
    movl 20(%esp), %ecx
    movl 24(%esp), %ebx
    movl $4, %eax
    sysenter
r_write:  
    addl $4, %esp
    popl %ebx
    popl %ebp
    movl %ebp, %esp
    ret

ENTRY(gettime)
    movl $10, %eax
    int $0x80
    ret
