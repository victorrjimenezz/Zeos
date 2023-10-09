/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>
#include <utils.h>
#include <io.h>
#include <sched.h>
#include <error_code.h>

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
}

unsigned int sys_gettime()
{
  return zeos_ticks;
}
