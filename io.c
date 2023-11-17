/*
 * io.c - 
 */

#include <io.h>

#include <types.h>

/**************/
/** Screen  ***/
/**************/

#define NUM_COLUMNS 80
#define NUM_ROWS    25

Byte x, y=19;

/* Read a byte from 'port' */
Byte inb (unsigned short port)
{
  Byte v;

  __asm__ __volatile__ ("inb %w1,%0":"=a" (v):"Nd" (port));
  return v;
}

inline void BreakLine()
{
    x = 0;
    Word *screen = (Word *)0xb8000;

    if (y == NUM_ROWS - 1)
    {
        for (int i = 0; i < NUM_ROWS - 1; ++i)
        {
            for (int j = 0; j < NUM_COLUMNS; ++j)
            {
                screen[(i * NUM_COLUMNS + j)] = screen[((i + 1) * NUM_COLUMNS + j)];
            }
        }
    }
    else
        ++y;

    for (int j = 0; j < NUM_COLUMNS; ++j)
    {
        screen[((NUM_ROWS - 1) * NUM_COLUMNS + j)] = ' ';
    }

}

void printccolor(char c, unsigned int console_color)
{
    __asm__ __volatile__ ( "movb %0, %%al; outb $0xe9" ::"a"(c)); /* Magic BOCHS debug: writes 'c' to port 0xe9 */
    if (c == '\n')
        BreakLine();
    else
    {
        Word ch = (Word) (c & 0x00FF) | console_color;
        Word *screen = (Word *)0xb8000;
        screen[(y * NUM_COLUMNS + x)] = ch;
        if (++x >= NUM_COLUMNS)
            BreakLine();
    }
}

void printc(char c)
{
    printccolor(c, CCWhite);
}

void printc_xy(Byte mx, Byte my, char c)
{
  Byte cx, cy;
  cx=x;
  cy=y;
  x=mx;
  y=my;
  printc(c);
  x=cx;
  y=cy;
}

void printk(char *string)
{
  int i;
  for (i = 0; string[i]; i++)
    printc(string[i]);
}