/*
 * entry.S - Entry point to system mode from user mode
 */

#include <asm.h>
#include <segment.h>
#include <error_code.h>


/**************************************************/
/**** Save & Restore ******************************/
/**                                              **/
/** When we change to privilege level 0 (kernel) **/
/** (through an interrupt, a system call, an     **/
/** exception ...) we must save the state of the **/
/** currently running task (save).               **/
/**                                              **/
/** Stack layout in 'systemCall':                **/
/**                                              **/
/**   0(%esp) - %edx    \                        **/
/**   4(%esp) - %ecx     |                       **/
/**   8(%esp) - %ebx     |                       **/
/**   C(%esp) - %esi     | Register saved        **/
/**  10(%esp) - %edi     |  by 'save'            **/
/**  14(%esp) - %ebp     |                       **/
/**  18(%esp) - %eax     |                       **/
/**  1C(%esp) - %ds      |                       **/
/**  20(%esp) - %es      |                       **/
/**  24(%esp) - %fs      |                       **/
/**  28(%esp) - %gs     /                        **/
/**  2C(%esp) - %eip    \                        **/
/**  30(%esp) - %cs      |                       **/
/**  34(%esp) - %eflags  |  Return context saved **/
/**  38(%esp) - %oldesp  |   by the processor.   **/
/**  3C(%esp) - %oldss  /                        **/
/**                                              **/
/**************************************************/

#define SAVE_ALL \
      pushl %gs; \
      pushl %fs; \
      pushl %es; \
      pushl %ds; \
      pushl %eax; \
      pushl %ebp; \
      pushl %edi; \
      pushl %esi; \
      pushl %ebx; \
      pushl %ecx; \
      pushl %edx; \
      movl $__KERNEL_DS, %edx;    \
      movl %edx, %ds;           \
      movl %edx, %es

#define RESTORE_ALL \
      popl %edx; \
      popl %ecx; \
      popl %ebx; \
      popl %esi; \
      popl %edi; \
      popl %ebp; \
      popl %eax; \
      popl %ds; \
      popl %es; \
      popl %fs; \
      popl %gs

#define EOI \
      movb $0x20, %al ; \
      outb %al, $0x20

ENTRY(keyboard_handler)
      SAVE_ALL
      EOI
      call keyboard_routine
      RESTORE_ALL
      iret

ENTRY(clock_handler)
      SAVE_ALL
      EOI
      call clock_routine
      RESTORE_ALL
      iret

ENTRY(segmentation_fault_handler)
     movl (%esp), %ecx
     movl 4(%esp), %edx
     pushl %ecx
     pushl %edx
     call segmentation_fault_routine

ENTRY(sys_call_handler)
      SAVE_ALL
      cmpl $0, %eax
      jl err
      cmpl $MAX_SYSCALL, %eax
      jg err
      call *sys_call_table(, %eax, 0x04)
      jmp fin
err:
      movl $ENOSYS, %eax
fin:
      movl %eax, 0x18(%esp)
      RESTORE_ALL
      iret

ENTRY(sysenter_handler)
      push $__USER_DS
      push %EBP // User stack address
      pushfl
      push $__USER_CS
      push 4(%EBP) // User return address
      SAVE_ALL
      cmpl $0, %EAX
      jl sysenter_err
      cmpl $MAX_SYSCALL, %EAX
      jg sysenter_err
      call *sys_call_table(, %EAX, 0x04)
      jmp sysenter_fin
sysenter_err:
      movl $ENOSYS, %EAX
sysenter_fin:
      movl %EAX, 0x18(%ESP)
      RESTORE_ALL
      movl (%ESP), %EDX // Return address
      movl 12(%ESP), %ECX // User stack address
      sti // Enable interrupts again
      sysexit

ENTRY(writeMSR)
      pushl %ebp
      movl %esp, %ebp
      movl 8(%ebp), %ecx
      movl 12(%ebp), %eax
      movl $0, %edx
      wrmsr
      popl %ebp
      ret