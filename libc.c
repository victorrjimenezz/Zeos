/*
 * libc.c 
 */

#include <libc.h>
#include <error_code.h>

int errno;

void itoa(int a, char* b)
{
    // If it's negative get the positive value and print a '-' sign.
    if (a & (1 << 31))
    {
        a = -a;
        b[0] = '-';
        ++b;
    }

    b[0] = '0';
    int index = 0;
    // Store each digit in reverse order.
    while (a)
    {
        b[index++] = a % 10 + '0';
        a /= 10;
    }

    // Reverse the array.
    char aux;
    for (int i = 0; i < index; i++)
    {
        aux = b[index - 1];
        b[index - 1] = b[i];
        b[i] = aux;
    }
}

int strlen(char *a)
{
  int i;
  
  i=0;
  
  while (a[i]!=0) i++;
  
  return i;
}

void perror()
{
    char buff[] = "\nERROR:    ";

    write(2, buff, strlen(buff));

    switch (errno)
    {
        case ENOSYS:
        {
            char missatge[] = "Aquest syscall no existeix, no implementat\n";
            write(2, missatge, strlen(missatge));
            break;
        }

        case EBNRTASK:
        {
            char missatge[] = "No more free tasks\n";
            write(2, missatge, strlen(missatge));
            break;
        }

        case EBADF:
        {
            char missatge[] = "File descriptor erroni\n";
            write(2, missatge, strlen(missatge));
            break;
        }
        case EACCES:
        {
            char missatge[] = "Permís denegat\n";
            write(2, missatge, strlen(missatge));
            break;
        }

        case NULLBUFF:
        {
            char missatge[] = "Buffer invalid\n";
            write(2, missatge, strlen(missatge));
            break;
        }

        case INVSIZE:
        {
            char missatge[] = "Tamany incorrecte del buffer\n";
            write(2, missatge, strlen(missatge));
            break;
        }
        default:
            break;
    }
}