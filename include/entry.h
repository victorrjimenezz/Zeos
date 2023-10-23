/*
 * entry.h - Definició del punt d'entrada de les crides al sistema
 */

#ifndef __ENTRY_H__
#define __ENTRY_H__

void keyboard_handler();
void clock_handler();
void segmentation_fault_handler();
void sys_call_handler();
void sysenter_handler();
void writeMSR(int index, void *value);
void fix_stack();

#endif  /* __ENTRY_H__ */
