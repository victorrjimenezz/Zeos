#include <asm.h>


ENTRY(write)
    pushl %ebp
    movl %esp, %ebp

    pushl %ebx

    movl 8(%ebp), %edx // Pas de paramatres
    movl 12(%ebp), %ecx
    movl 16(%ebp), %ebx

    pushl $r_write // Adreça de retorn 
    pushl %ebp // salvem el ebp d'aquesta funció 
    movl %esp, %ebp // fiquem a ebp el esp per despres tornar
    movl $4, %eax
    sysenter
r_write:
    cmpl $0, %eax // si eax < 0 hi ha un error
    jge w_no_err // else 

    //movl %eax, errno
    movl $-1, %eax

w_no_err:
    popl %ebp
    addl $4, %esp
    popl %ebx
    movl %ebp, %esp
    popl %ebp
    ret

ENTRY(gettime)
    movl $10, %eax
    int $0x80

    cmpl $0, %eax // si eax < 0 hi ha un error
    jge gt_no_err // else 

    //movl %eax, errno
    movl $-1, %eax
gt_no_err:
    ret
