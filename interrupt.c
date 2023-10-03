/*
 * interrupt.c -
 */
#include <types.h>
#include <interrupt.h>
#include <segment.h>
#include <hardware.h>
#include <io.h>
#include <entry.h>

#include <zeos_interrupt.h>

Gate idt[IDT_ENTRIES];
Register    idtR;
extern unsigned int zeos_ticks;

char char_map[] =
{
  '\0','\0','1','2','3','4','5','6',
  '7','8','9','0','\'','�','\0','\0',
  'q','w','e','r','t','y','u','i',
  'o','p','`','+','\0','\0','a','s',
  'd','f','g','h','j','k','l','�',
  '\0','�','\0','�','z','x','c','v',
  'b','n','m',',','.','-','\0','*',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','7',
  '8','9','-','4','5','6','+','1',
  '2','3','0','\0','\0','\0','<','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0'
};

void setInterruptHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
  /***********************************************************************/
  /* THE INTERRUPTION GATE FLAGS:                          R1: pg. 5-11  */
  /* ***************************                                         */
  /* flags = x xx 0x110 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);
  flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}

void setTrapHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
  /***********************************************************************/
  /* THE TRAP GATE FLAGS:                                  R1: pg. 5-11  */
  /* ********************                                                */
  /* flags = x xx 0x111 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);

  //flags |= 0x8F00;    /* P = 1, D = 1, Type = 1111 (Trap Gate) */
  /* Changed to 0x8e00 to convert it to an 'interrupt gate' and so
     the system calls will be thread-safe. */
  flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}

void setIdt()
{
  /* Program interrups/exception service routines */
  idtR.base  = (DWord)idt;
  idtR.limit = IDT_ENTRIES * sizeof(Gate) - 1;

  set_handlers();

  /* ADD INITIALIZATION CODE FOR INTERRUPT VECTOR */
  setInterruptHandler(33, &keyboard_handler, 0);
  setInterruptHandler(32, &clock_handler, 0);
  setInterruptHandler(14, &segmentation_fault_handler, 3);
  setTrapHandler(0x80, &sys_call_handler, 3);

  set_idt_reg(&idtR);
}

void segmentation_fault_routine(int eip, int error_code)
{
    char message[] = "There was a page fault with error code ";
    for (int i = 0; message[i] != '\0'; ++i)
        printccolor(message[i], CCRed);

    // Store the EIP as a hex.
    int index = 0;
    char hex_value[32] = "";
    while (error_code)
    {
        hex_value[index++] = "0123456789"[error_code % 10];
        error_code /= 10;
    }

    // Write the error_code value to the screen.
    while (index > 0)
        printccolor(hex_value[--index], CCRed);

    printccolor(' ', CCRed);
    printccolor('a', CCRed);
    printccolor('t', CCRed);
    printccolor(':', CCRed);
    printccolor(' ', CCRed);
    printccolor('0', CCRed);
    printccolor('x', CCRed);

    index = 0;
    while (eip)
    {
        hex_value[index++] = "0123456789ABCDEF"[eip % 16];
        eip /= 16;
    }

    // Write the EIP value to the screen.
    while (index > 0)
        printccolor(hex_value[--index], CCRed);

    while (1);
}

void keyboard_routine()
{
  char character_in = inb(0x60);
  // Make (key pressed) => 7-th bit == 0.
  if (!(character_in & 0x80))
  {
     if (character_in == 0x1c)
         printc('\n');
     else
         printc(char_map[character_in]);
  }
}

void clock_routine()
{
  zeos_show_clock();
  zeos_ticks++;
}