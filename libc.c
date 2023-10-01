/*
 * libc.c 
 */

#include <libc.h>

#include <types.h>

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