/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>
#include <utils.h>
#include <io.h>
#include <sched.h>
#include <error_code.h>
#include "mm.h"

#define LECTURA 0
#define ESCRIPTURA 1
#define ERROR 2

unsigned int zeos_ticks = 0;
extern struct list_head freequeue;
extern struct list_head readyqueue;

void update_stats(int entering) // El pramatre actua com a boolea i inidica si estem entrant o sortint de sys
{
    if (entering)
    {
        current()->stadistics.user_ticks += get_ticks()-current()->stadistics.elapsed_total_ticks;
        current()->stadistics.elapsed_total_ticks = get_ticks(); 
    }
    else
    {
        current()->stadistics.system_ticks += get_ticks()-current()->stadistics.elapsed_total_ticks;
        current()->stadistics.elapsed_total_ticks = get_ticks(); 
    }
}

int check_fd(int fd, int permissions)
{
  if (fd != ESCRIPTURA && fd != ERROR) return EBADF;
  if (permissions != ESCRIPTURA) return EACCES;

  return 0;
}

int sys_ni_syscall()
{
	return ENOSYS;
}

int sys_write(int fd, char * buffer, int size)
{
    update_stats(1);
    int ret = check_fd(fd, ESCRIPTURA);

    if (ret < 0)
        return ret;

    if (buffer == NULL)
        return NULLBUFF;

    if (size < 0)
        return INVSIZE;

    char local_buffer[1024];
    int total_bytes = 0;
    while (size > 0)
    {
    	int current_bytes = size < 1024 ? size : 1024;
        copy_from_user(buffer, local_buffer, current_bytes);
        
    	if (fd == ERROR)
            total_bytes += sys_write_error(local_buffer, current_bytes);
        else
    	    total_bytes += sys_write_console(local_buffer, current_bytes);
    	    
        buffer += 1024;
        size -= 1024;
    }
    
    update_stats(0);
    return total_bytes;
}

int sys_getpid()
{
	return current()->PID;
}

int ret_from_fork()
{
    return 0;
}

int sys_fork()
{
  update_stats(1);
  extern unsigned int switching_enabled;
  switching_enabled = 0;

  if (list_empty(&freequeue))
  {
      switching_enabled = 1;
      return EBNRTASK;
  }

  struct task_struct * parent_task = current();

  struct list_head *empty_process = list_pop(&freequeue);
  union task_union * child_union = (union task_union *) list_head_to_task_struct(empty_process);
  child_union->task = *parent_task;
  child_union->task.PID = (zeos_ticks ^ 0xDEADBEEF) & 0x7FFFFFFF;
  allocate_DIR(&child_union->task);
  page_table_entry * process_child_PT = get_PT(&child_union->task);
  page_table_entry * process_parent_PT = get_PT(parent_task);

  {
    int pag;
    int new_ph_pag;

    for (pag=0;pag<NUM_PAG_CODE;pag++)
    {
        process_child_PT[PAG_LOG_INIT_CODE+pag].entry = process_parent_PT[PAG_LOG_INIT_CODE+pag].entry;
        process_child_PT[PAG_LOG_INIT_CODE+pag].bits.pbase_addr = process_parent_PT[PAG_LOG_INIT_CODE+pag].bits.pbase_addr;
        process_child_PT[PAG_LOG_INIT_CODE+pag].bits.user = process_parent_PT[PAG_LOG_INIT_CODE+pag].bits.user;
        process_child_PT[PAG_LOG_INIT_CODE+pag].bits.present = process_parent_PT[PAG_LOG_INIT_CODE+pag].bits.present;
    }

    for (pag=0;pag<NUM_PAG_DATA;pag++)
    {
        new_ph_pag=alloc_frame();
        process_child_PT[PAG_LOG_INIT_DATA+pag].entry = 0;
        process_child_PT[PAG_LOG_INIT_DATA+pag].bits.pbase_addr = new_ph_pag;
        process_child_PT[PAG_LOG_INIT_DATA+pag].bits.user = 1;
        process_child_PT[PAG_LOG_INIT_DATA+pag].bits.rw = 1;
        process_child_PT[PAG_LOG_INIT_DATA+pag].bits.present = 1;

        set_ss_pag(process_parent_PT, PAG_LOG_INIT_CODE + NUM_PAG_CODE + pag, new_ph_pag);
    }
  }

  copy_data((void *) (PAG_LOG_INIT_DATA * PAGE_SIZE), (void *) ((PAG_LOG_INIT_CODE + NUM_PAG_CODE) * PAGE_SIZE), PAGE_SIZE * NUM_PAG_DATA);

  for (int pag=0;pag<NUM_PAG_DATA;pag++)
       del_ss_pag(process_parent_PT, PAG_LOG_INIT_CODE + NUM_PAG_CODE + pag);

  set_cr3(parent_task->dir_pages_baseAddr);

  child_union->task.esp = (int) &child_union->stack[KERNEL_STACK_SIZE - 0x13];

  child_union->stack[KERNEL_STACK_SIZE - 0x13] = 0;
  child_union->stack[KERNEL_STACK_SIZE - 0x12] = (int) &ret_from_fork;

  for (int i = 0; i < 0x11; i++)
      child_union->stack[KERNEL_STACK_SIZE - 1 - i] = ((union task_union *) parent_task)->stack[KERNEL_STACK_SIZE - 1 - i];


  list_add(&child_union->task.list, &readyqueue);

  child_union->task.stadistics.user_ticks = 0;
  child_union->task.stadistics.system_ticks = 0;
  child_union->task.stadistics.blocked_ticks = 0;
  child_union->task.stadistics.ready_ticks = 0;
  child_union->task.stadistics.elapsed_total_ticks = get_ticks();
  child_union->task.stadistics.total_trans = 0;
  child_union->task.stadistics.remaining_ticks = 0;

  update_stats(0);
  switching_enabled = 1;
  return child_union->task.PID;
}

void sys_exit()
{
    update_stats(1);
    extern unsigned int switching_enabled;
    switching_enabled = 0;

    struct task_struct * current_process = current();

    extern struct list_head freequeue;
    update_process_state_rr(current_process, &freequeue);
    current_process->PID = 0;
    current_process->quantum = 0;

    page_table_entry * page_tage = get_PT(current_process);

    /* CODE */
    for (int pag = 0; pag < NUM_PAG_CODE; pag++)
    {
        free_frame(page_tage[PAG_LOG_INIT_DATA+pag].bits.pbase_addr);
        page_tage[PAG_LOG_INIT_CODE+pag].entry = 0;
        page_tage[PAG_LOG_INIT_CODE+pag].bits.user = 0;
        page_tage[PAG_LOG_INIT_CODE+pag].bits.present = 0;
    }

    /* DATA */
    for (int pag = 0; pag < NUM_PAG_CODE; pag++)
    {
        free_frame(page_tage[PAG_LOG_INIT_DATA+pag].bits.pbase_addr);
        page_tage[PAG_LOG_INIT_DATA+pag].entry = 0;
        page_tage[PAG_LOG_INIT_DATA+pag].bits.user = 0;
        page_tage[PAG_LOG_INIT_DATA+pag].bits.rw = 0;
        page_tage[PAG_LOG_INIT_DATA+pag].bits.present = 0;
    }

    extern struct list_head readyqueue;
    union task_union * next_task = (union task_union *) list_head_to_task_struct(list_pop(&readyqueue));
    extern unsigned int current_quantum;
    current_quantum = get_quantum(&next_task->task);
    update_stats(0);
    switching_enabled = 1;
    task_switch(next_task);
}

unsigned int sys_gettime()
{
  return zeos_ticks;
}

int sys_get_stats(int pid, struct stats* st)
{
  update_stats(1);
  if (!st) return -1; // Es pot canviar per un error que indiqui que el punter es invalid  
  extern union task_union task[NR_TASKS]; /* Vector de tasques */
  
  int found = 0, i = 0;
  union task_union* target;

  while (!found) 
  {
    target = &task[i++];
    if (target->task.PID == pid) found = 1;
  }
  
  if (!found) { printk("not found\n"); return -1; }

  struct task_struct* current = (struct task_struct*)target;

  st->user_ticks = current->stadistics.user_ticks;
  st->system_ticks = current->stadistics.system_ticks;
  st->blocked_ticks = current->stadistics.blocked_ticks;
  st->ready_ticks = current->stadistics.ready_ticks;
  st->elapsed_total_ticks = current->stadistics.elapsed_total_ticks;
  st->total_trans = current->stadistics.total_trans;
  st->remaining_ticks = current->stadistics.remaining_ticks;

  update_stats(0);
  return 0;
}