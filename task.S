#include <asm.h>
#include <segment.h>

ENTRY(task_switch)
      pushl %ebp
      movl %esp, %ebp
      pushl %esi
      pushl %edi
      pushl %ebx
      movl 8(%ebp), %eax
      pushl %eax
      call inner_task_switch
      addl $4, %esp
      popl %ebx
      popl %edi
      popl %esi
      movl %ebp, %esp
      popl %ebp
      ret
      
ENTRY(ebp_switch)
      pushl %ebp
      movl %esp, %ebp
      movl 8(%ebp), %eax
      movl %ebp, (%eax)
      movl 12(%ebp), %eax
      movl (%eax), %ebp
      movl %ebp, %esp
      popl %ebp
      ret
