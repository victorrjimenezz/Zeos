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

int sys_fork()
{
  int PID=-1;

  // creates the child process
  
  return PID;
}

void sys_exit()
{
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
    switching_enabled = 1;
    task_switch(next_task);
}

unsigned int sys_gettime()
{
  return zeos_ticks;
}
