/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>
#include <error_code.h>

#define LECTURA 0
#define ESCRIPTURA 1

int check_fd(int fd, int permissions)
{
  if (fd!=1) return EBADF;
  if (permissions!=ESCRIPTURA) return EACCES;
  return 0;
}

int sys_ni_syscall()
{
	return ENOSYS;
}

int sys_write(int fd, char * buffer, int size)
{
    int ret = check_fd (fd, ESCRIPTURA);

    if (ret < 0)
        return ret;

    if (buffer == NULL)
        return NULLBUFF;

    if (size < 0)
        return INVSIZE;

    char local_buffer[19];
    copy_from_user(buffer, local_buffer, size);
    
    return sys_write_console(local_buffer, size);
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
}
