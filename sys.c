/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>
#include <utils.h>
#include <io.h>
#include <sched.h>
#include <error_code.h>
#include <mm.h>

#define LECTURA 0
#define ESCRIPTURA 1
#define ERROR 2

unsigned int zeos_ticks = 0;

int ret_from_fork() 
{
  return 0;
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
  extern struct list_head freequeue;
  extern struct list_head readyqueue;

  struct list_head* head = list_pop(&freequeue);  // He d'assignar el head al task struct del child?
  
  if (!head) { 
    return ENOPCB; // Error no hi ha prou memoria
  }

  union task_union* parent = (union task_union*)current();
  union task_union* child = (union task_union*)list_head_to_task_struct(head);

  copy_data(parent, child, sizeof(union task_union));   // copio el union també copio el stack i el PCB

  if (!allocate_DIR(&child->task)) {
    list_add_tail(head, &freequeue); 
    return ENOMEM; // Busquem si hi ha prou memoria pel PCB del fill si no n'hi ha Retornem error
  }

  int pages[NUM_PAG_DATA];   // Search phisical pages to allocate the .data
  for (int i = 0; i < NUM_PAG_DATA; ++i) {
    pages[i] = alloc_frame();
    if (pages[i] == -1) { // Això passarà quan no hi hagi memoria
      for (int j = i; j >= 0; --j) free_frame(pages[j]); // Alliberem les pagines que ja haviem reservat
      list_add_tail(head, &freequeue);  // Si falla tornem a ficar el PCB a la ready_queue
      return ENOMEM; 
    }
  }

  for (unsigned int i = 0; i < NUM_PAG_KERNEL; ++i) // Fem que les entrades de la PT del fill apuntin al codi de sys del pare
                                                    // es comença per la adreça 0(=i) que és on està el kernel
    set_ss_pag(child->task.dir_pages_baseAddr, i, get_frame(parent->task.dir_pages_baseAddr, i));

  for (unsigned int i = 0; i < NUM_PAG_DATA; ++i) // Enlloc de assignar les pagines del pare assignem les noves que hem reservat
    set_ss_pag(child->task.dir_pages_baseAddr, i + PAG_LOG_INIT_DATA, pages[i]);

  for (unsigned int i = 0; i < NUM_PAG_CODE; ++i) // El mateix que abans però amb el codi
    set_ss_pag(child->task.dir_pages_baseAddr, i + PAG_LOG_INIT_CODE, get_frame(parent->task.dir_pages_baseAddr, i + PAG_LOG_INIT_CODE));

  for (unsigned int i = 0; i < NUM_PAG_DATA; ++i) // Enlloc de assignar les pagines del pare assignem les noves que hem reservat
    set_ss_pag(parent->task.dir_pages_baseAddr, 0x200 + i, pages[i]); // Adreça hardcoded per veure si funciona (no va)
    //set_ss_pag(parent->task.dir_pages_baseAddr, i + PAG_LOG_INIT_CODE + NUM_PAG_CODE, pages[i]); // Assignem les pagines fisiques de dades del fill al par

  //copy_data((int*)L_USER_START, (int*)L_USER_START, NUM_PAG_DATA*4096); // Copiant ho al mateix lloc si que va
  copy_data((int*)L_USER_START, 0x200000, NUM_PAG_DATA*4096); // Adreça hardcoded per veure si funciona (no va)
  //copy_data((int*)L_USER_START, (int*)((NUM_PAG_CODE + NUM_PAG_DATA)*4096 + L_USER_START), NUM_PAG_DATA*4096); // Codi que creiem que esta be

  for (unsigned int i = 0; i < NUM_PAG_DATA; ++i) // Eliminem les entrades que hem afegit a la TP del pare
    //del_ss_pag(parent->task.dir_pages_baseAddr, i + PAG_LOG_INIT_CODE + NUM_PAG_CODE);
    del_ss_pag(parent->task.dir_pages_baseAddr, 0x200 + i);

  set_cr3(parent->task.dir_pages_baseAddr);

  child->task.PID = sys_getpid() + 1;  // Faig aixo pq lo altre em dona problemes de tipus

  child->task.esp = (int)(child->stack + (14 + 3 + 5)*4);  // 14 del saveAll, 3 dels returns que forcem i 5 del ctx hw

  unsigned int* esp = (unsigned int*)child->task.esp; // Ho de fer així pq amb un casting no em deixa

  esp[0] = 0;
  esp[-4] = (unsigned int)ret_from_fork;

  list_add_tail(head, &readyqueue);
 
  return PID;
}

void sys_exit()
{  
}

unsigned int sys_gettime()
{
  return zeos_ticks;
}

int sys_get_stats(int pid, struct stats *st)
{
  
}