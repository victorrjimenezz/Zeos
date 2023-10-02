#include <libc.h>

char buff[24];

int pid;

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */


    int current_time = gettime();
    for (int i = 0; i < 2000000; ++i)
    {
        current_time = gettime();
    }

    char test_buffer[32];
    itoa(current_time, test_buffer);
    int ret = write(1, test_buffer, strlen(test_buffer));
    write(1, test_buffer, strlen(test_buffer));
  while(1) { }
}
