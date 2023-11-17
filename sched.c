/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>
#include "entry.h"
#include "utils.h"
#include <task.h>

union task_union task[NR_TASKS] __attribute__((__section__(".data.task")));

struct task_struct * idle_task;

extern struct list_head freequeue;
extern struct list_head readyqueue;
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
    set_quantum(&task_union->task, 1);
    idle_task = (struct task_struct *) task_union;
    idle_task->PID = 0;
    allocate_DIR(idle_task);
    idle_task->esp = (int) &task_union->stack[KERNEL_STACK_SIZE - 2];
    task_union->stack[KERNEL_STACK_SIZE - 2] = 0;
    task_union->stack[KERNEL_STACK_SIZE - 1] = (int) &cpu_idle;

    task_union->task.stadistics.user_ticks = 0;
    task_union->task.stadistics.system_ticks = 0;
    task_union->task.stadistics.blocked_ticks = 0;
    task_union->task.stadistics.ready_ticks = 0;
    task_union->task.stadistics.elapsed_total_ticks = get_ticks();
    task_union->task.stadistics.total_trans = 0;
    task_union->task.stadistics.remaining_ticks = 0;
    
    list_add(&task_union->task.list, &readyqueue);
}

void init_task1(void)
{
    struct list_head *empty_process = list_pop(&freequeue);
    union task_union * task_union = (union task_union *) list_head_to_task_struct(empty_process);
    set_quantum(&task_union->task, 100);
    task_union->task.PID = 1;
    allocate_DIR(&task_union->task);
    set_user_pages(&task_union->task);

    tss.esp0 = (DWord) &task_union->stack[KERNEL_STACK_SIZE];
    writeMSR(0x175, &task_union->stack[KERNEL_STACK_SIZE]);

    set_cr3(task_union->task.dir_pages_baseAddr);

    task_union->task.stadistics.user_ticks = 0;
    task_union->task.stadistics.system_ticks = 0;
    task_union->task.stadistics.blocked_ticks = 0;
    task_union->task.stadistics.ready_ticks = 0;
    task_union->task.stadistics.elapsed_total_ticks = get_ticks();
    task_union->task.stadistics.total_trans = 0;
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
    return list_entry( l, struct task_struct, list);
}

void inner_task_switch(union task_union * new)
{
    struct task_struct * current_tu = current();

    tss.esp0 = (DWord) &new->stack[KERNEL_STACK_SIZE];
    writeMSR(0x175, &new->stack[KERNEL_STACK_SIZE]);
    if (current_tu->task.pid != new->task.pid) // Optimització comentada a classe per threads
        set_cr3(new->task.dir_pages_baseAddr);

    ebp_switch(&current_tu->esp, &new->task.esp);
}

unsigned int current_quantum = 100;

void sched_next_rr()
{
    union task_union * task_union = (union task_union *) list_head_to_task_struct(list_pop(&readyqueue));

    if (task_union->task.PID == 0 && !list_empty(&readyqueue))
    {
        list_add_tail(&task_union->task.list, &readyqueue);
        task_union = (union task_union *) list_head_to_task_struct(list_pop(&readyqueue));
    }

    current_quantum = get_quantum(&task_union->task);
    task_union->task.stadistics.ready_ticks += get_ticks()-task_union->task.stadistics.elapsed_total_ticks;
    task_union->task.stadistics.remaining_ticks = current_quantum;
    task_union->task.stadistics.elapsed_total_ticks = get_ticks();
    ++task_union->task.stadistics.total_trans;
    task_switch(task_union);
}

void update_process_state_rr(struct task_struct *t, struct list_head *dest)
{
    struct task_struct* actual = current();
    actual->stadistics.user_ticks += get_ticks()-current()->stadistics.elapsed_total_ticks;
    actual->stadistics.elapsed_total_ticks = get_ticks(); // Aquí el procés passa de systema a ready

    if (in_list(&t->list))
        list_del(&t->list);

    list_add_tail(&t->list, dest);
}

int needs_sched_rr()
{
    return !current_quantum;
}

void update_sched_data_rr()
{
    --current_quantum;
    current()->stadistics.remaining_ticks = current_quantum;
}

void schedule()
{
    if (needs_sched_rr())
    {
        if (!list_empty(&readyqueue))
        {
            update_process_state_rr(current(), &readyqueue);
            sched_next_rr();
        }
        else
            current_quantum = current()->quantum;
    }
}

unsigned int get_quantum(struct task_struct *t)
{
    return t->quantum;
}

void set_quantum(struct task_struct *t, int new_quantum)
{
    t->quantum = new_quantum;
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

void thread_wrapper(void (*func)(void*), void *paramaters) 
{
    (*func)(paramaters);
    exit();
}