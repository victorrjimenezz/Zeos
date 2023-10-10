/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>
#include "entry.h"

union task_union task[NR_TASKS];
struct list_head freequeue;
struct list_head readyqueue;
struct task_struct * idle_task;

  __attribute__((__section__(".data.task")));

#if 0
struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return list_entry( l, struct task_struct, list);
}
#endif

extern struct list_head blocked;


/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry * get_DIR (struct task_struct *t) 
{
	return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry * get_PT (struct task_struct *t) 
{
	return (page_table_entry *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr))<<12);
}


int allocate_DIR(struct task_struct *t) 
{
	int pos;

	pos = ((int)t-(int)task)/sizeof(union task_union);

	t->dir_pages_baseAddr = (page_table_entry*) &dir_pages[pos]; 

	return 1;
}

void cpu_idle(void)
{
	__asm__ __volatile__("sti": : :"memory");

	while(1)
	{
	;
	}
}

void init_idle (void)
{
    struct list_head *empty_process = list_pop(&freequeue);
    union task_union * task_union = (union task_union *) list_head_to_task_struct(empty_process);
    idle_task = &task_union->task;
    task_union->task.PID = 0;
    allocate_DIR(&task_union->task);
    char * stack_ptr = (char *)task_union->stack;
    int * stack_int_ptr = (int *)(&stack_ptr[sizeof(struct task_struct)]);
    stack_int_ptr[0] = (int) &cpu_idle;
    stack_int_ptr[1] = 0;
    task_union->task.stack_ptr = (int) &stack_int_ptr[2];
}

void init_task1(void)
{
    struct list_head *empty_process = list_pop(&freequeue);
    union task_union * task_union = (union task_union *) list_head_to_task_struct(empty_process);
    task_union->task.PID = 1;
    allocate_DIR(&task_union->task);
    set_user_pages(&task_union->task);

    char * stack_ptr = (char *)task_union->stack;
    int * stack_int_ptr = (int *)(&stack_ptr[sizeof(struct task_struct)]);
    task_union->task.stack_ptr = (int) &stack_int_ptr[0];

    tss.esp0 = (DWord) &task_union->stack[KERNEL_STACK_SIZE - 256];
    writeMSR(0x175, &task_union->stack[KERNEL_STACK_SIZE - 256]);


    set_cr3(task_union->task.dir_pages_baseAddr);
}

void init_sched()
{
	INIT_LIST_HEAD(&freequeue);
	INIT_LIST_HEAD(&readyqueue);
	for (int i = 0; i < NR_TASKS; ++i)
	{
	     list_add(&task[i].task.list, &freequeue);
	}
}

struct task_struct *list_head_to_task_struct(struct list_head *l)
{
	char * address = (char*)l;
	address -= sizeof(struct list_head) + 8;
	return (struct task_struct *) address;
}

struct task_struct* current()
{
  int ret_value;
  
  __asm__ __volatile__(
  	"movl %%esp, %0"
	: "=g" (ret_value)
  );
  return (struct task_struct*)(ret_value&0xfffff000);
}

