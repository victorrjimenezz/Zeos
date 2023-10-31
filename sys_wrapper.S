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

    movl %eax, errno
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

    movl %eax, errno
    movl $-1, %eax
gt_no_err:
    ret

ENTRY(fork)
    movl $2, %eax // 2 es la syscall id
    int $0x80

    cmpl $0, %eax // si eax < 0 hi ha un error
    jge fk_no_err // else 

    movl %eax, errno
    movl $-1, %eax
fk_no_err:
    ret

ENTRY(getpid)
    movl $20, %eax // 20 es la syscall id
    int $0x80

    cmpl $0, %eax // si eax < 0 hi ha un error
    jge gp_no_err // else 

    movl %eax, errno
    movl $-1, %eax
gp_no_err:
    ret

ENTRY(get_stats)
    pushl %ebp
    movl %esp, %ebp

    pushl %ebx

    movl 8(%ebp), %edx // Pas de paramatres
    movl 12(%ebp), %ecx

    pushl $r_get_stats // Adreça de retorn 
    pushl %ebp // salvem el ebp d'aquesta funció 
    movl %esp, %ebp // fiquem a ebp el esp per despres tornar
    movl $35, %eax
    sysenter
r_get_stats:
    cmpl $0, %eax // si eax < 0 hi ha un error
    jge gs_no_err // else 

    movl %eax, errno
    movl $-1, %eax

gs_no_err:
    popl %ebp
    addl $4, %esp
    popl %ebx
    movl %ebp, %esp
    popl %ebp
    ret
