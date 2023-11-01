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

  if (list_empty(&freequeue))
  {
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
  return child_union->task.PID;
}

void sys_exit()
{
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
    task_switch(next_task);
}

unsigned int sys_gettime()
{
  return zeos_ticks;
}
